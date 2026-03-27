#include <cassert>
#include <atomic>
#include <thread>
#include <utility>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-core/math/math.hpp"
#include "litl-core/job/job.hpp"
#include "litl-core/job/jobPool.hpp"

namespace LITL::Core
{
    namespace
    {
        constexpr uint8_t GlobalPoolId = 255;
    }

    using JobAllocateResult = std::pair<Job*, uint32_t>;

    /// <summary>
    /// A thread-specific Job pool.
    /// 
    /// By default we allocate Jobs from here. If the thread-specific pool is full
    /// then the allocator should fallback to the shared job pool. 
    /// 
    /// By using a thread-specific pool we gain effectively free job allocation of the majority 
    /// of jobs as it eliminates the contention on the atomic indexer found in the general pool.
    /// </summary>
    class PerThreadJobPool
    {
    public:

        /// <summary>
        /// Allocates the raw Job instance in the per-thread pool.
        /// The owning JobPool wraps it into a JobHandle.
        /// </summary>
        /// <param name="version"></param>
        /// <returns></returns>
        [[nodiscard]] JobAllocateResult allocate(uint32_t version) noexcept
        {
            if ((m_currOffset + sizeof(Job)) > JobPoolBufferSize)
            {
                return { nullptr, 0 };
            }

            // The buffer is already allocated, just instantiate a new job inside it.
            //auto* job = new (m_buffer + m_currOffset) Job { .version = version };
            uint32_t jobIndex = m_currOffset / sizeof(Job);
            Job* job = std::construct_at(reinterpret_cast<Job*>(m_buffer + m_currOffset));
            job->version = version;

            m_currOffset += sizeof(Job);

            return { job, jobIndex };
        }

        void reset() noexcept
        {
            // No clear, no destructors, etc. needed. Just reset the offset into the buffer.
            m_currOffset = 0;
        }

        [[nodiscard]] Job* get(uint32_t index)
        {
            const auto offset = index * sizeof(Job);
            assert(offset < JobPoolBufferSize);
            return reinterpret_cast<Job*>(m_buffer + offset);
        }

    private:

        alignas(Constants::cache_line_size) std::byte m_buffer[JobPoolBufferSize];
        uint32_t m_currOffset = 0;
    };

    /// <summary>
    /// The thread-agnostic Job pool.
    /// 
    /// When a thread-specific pool is full, we fallback to allocate from here.
    /// It is (slightly) slower but is not limited in size like the thread-specific pools.
    /// </summary>
    class GlobalJobPool
    {
    public:

        GlobalJobPool()
        {
            m_blocks.push_back(std::make_unique<Block>());
        }

        /// <summary>
        /// Allocates the raw Job instance in the global pool.
        /// The owning JobPool wraps it into a JobHandle.
        /// </summary>
        /// <param name="version"></param>
        /// <returns></returns>
        [[nodiscard]] JobAllocateResult allocate(uint32_t version) noexcept
        {
            Job* job = nullptr;

            auto blockIndex = m_currentBlock.load(std::memory_order_relaxed);
            auto jobIndex = 0;

            auto* block = m_blocks[blockIndex].get();
            auto offset = block->currOffset.fetch_add(sizeof(Job), std::memory_order_relaxed);

            if ((offset + sizeof(Job)) <= JobPoolBufferSize)
            {
                // Room in the current block, so allocate from there.
                //job = new (block->buffer + offset) Job{ .version = version };
                job = std::construct_at(reinterpret_cast<Job*>(block->buffer + offset));
                jobIndex = offset / sizeof(Job);
            }
            else
            {
                // Block is out of room. Move to next block.
                blockIndex = m_currentBlock.fetch_add(1, std::memory_order_relaxed) + 1;

                if (blockIndex >= m_blocks.size())
                {
                    m_blocks.push_back(std::make_unique<Block>());
                }

                block = m_blocks[blockIndex].get();
                block->currOffset.store(sizeof(Job), std::memory_order_relaxed);

                //job = new (block->buffer + 0) Job{ .version = version };
                job = std::construct_at(reinterpret_cast<Job*>(block->buffer));
                jobIndex = 0;
            }

            job->version = version;

            return { job, ((blockIndex * JobPoolCount) + jobIndex) };
        }

        void reset() noexcept
        {
            for (auto& block : m_blocks)
            {
                // No clear, no destructors, etc. needed. Just reset the offset into the block.
                block->currOffset.store(0, std::memory_order_relaxed);
            }

            m_currentBlock.store(0, std::memory_order_relaxed);
        }

        Job* get(uint32_t index)
        {
            const auto block = index / JobPoolCount;
            const auto blockIndex = index % JobPoolCount;
            const auto blockOffset = blockIndex * sizeof(Job);

            assert(block < m_blocks.size());
            assert(blockOffset < JobPoolBufferSize);

            return reinterpret_cast<Job*>((m_blocks[block]->buffer) + blockOffset);
        }

    private:

        struct Block
        {
            alignas(Constants::cache_line_size) std::byte buffer[JobPoolBufferSize];
            std::atomic<uint32_t> currOffset;
        };

        std::vector<std::unique_ptr<Block>> m_blocks;
        std::atomic<uint32_t> m_currentBlock;
    };

    struct JobPool::Impl
    {
        uint32_t version{ 0 };
        GlobalJobPool globalPool;
        std::vector<std::unique_ptr<PerThreadJobPool>> localPools;
    };

    JobPool::JobPool(uint32_t threadCount)
        : m_pImpl(std::make_unique<Impl>())
    {
        threadCount = Math::clamp((threadCount > 0 ? threadCount : std::thread::hardware_concurrency() - 1), 1ul, 32ul);
        m_pImpl->localPools.reserve(threadCount);

        for (auto i = 0; i < threadCount; ++i)
        {
            m_pImpl->localPools.push_back(std::make_unique<PerThreadJobPool>());
        }
    }

    JobPool::~JobPool()
    {
        sync();
    }

    JobHandle JobPool::createJob(uint32_t threadIndex, Job::JobFunc func, void* externalData) const noexcept
    {
        assert((threadIndex + 1) < GlobalPoolId);

        JobAllocateResult result{ nullptr, 0 };
        auto poolIndex = 0u;

        if (threadIndex < m_pImpl->localPools.size())
        {
            result = m_pImpl->localPools[threadIndex]->allocate(m_pImpl->version);
            poolIndex = threadIndex + 1;        // offset by 1 to keep JobHandle(0, 0) as null/empty handle
        }

        if (result.first == nullptr)
        {
            result = m_pImpl->globalPool.allocate(m_pImpl->version);
            poolIndex = GlobalPoolId;
        }

        assert(result.first != nullptr);

        result.first->func = func;
        result.first->data = externalData;

        return JobHandle{ static_cast<uint8_t>(poolIndex), result.second};
    }

    bool JobPool::addDependency(JobHandle dependent, JobHandle dependency) const noexcept
    {
        auto dependentJob = resolve(dependent);
        auto dependencyJob = resolve(dependency);

        if ((dependentJob == nullptr) || (dependencyJob == nullptr) || (dependentJob->version != dependencyJob->version))
        {
            return false;
        }

        auto dependentIndex = dependencyJob->dependentsCount.fetch_add(1, std::memory_order_relaxed);

        if (dependentIndex >= dependencyJob->dependents.size())
        {
            // The dependency is already maxed out in dependents.
            return false;
        }

        dependencyJob->dependents[dependentIndex] = dependent;
        dependentJob->dependencyCount.fetch_add(1, std::memory_order_relaxed);

        return true;
    }

    void JobPool::sync() const noexcept
    {
        m_pImpl->globalPool.reset();

        for (auto& localPool : m_pImpl->localPools)
        {
            localPool->reset();
        }

        m_pImpl->version++;
    }

    uint32_t JobPool::version() const noexcept
    {
        return m_pImpl->version;
    }

    Job* JobPool::resolve(JobHandle handle) const noexcept
    {
        if (handle.isNull())
        {
            return nullptr;
        }

        const auto pool = handle.pool();
        const auto job = handle.job();

        if (pool != GlobalPoolId)
        {
            return m_pImpl->localPools[pool - 1]->get(job);
        }
        else
        {
            return m_pImpl->globalPool.get(job);
        }
    }
}