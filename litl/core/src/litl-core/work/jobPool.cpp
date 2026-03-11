#include <cassert>
#include <atomic>
#include <thread>
#include <vector>

#include "litl-core/alignment.hpp"
#include "litl-core/math/math.hpp"
#include "litl-core/work/job.hpp"
#include "litl-core/work/jobPool.hpp"

namespace LITL::Core
{
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
        [[nodiscard]] Job* allocate(uint32_t version) noexcept
        {
            if ((m_currOffset + sizeof(Job)) > JobPoolBufferSize)
            {
                return nullptr;
            }

            // The buffer is already allocated, just instantiate a new job inside it.
            auto* job = new (m_buffer + m_currOffset) Job { .version = version };

            m_currOffset += sizeof(Job);

            return job;
        }

        void reset() noexcept
        {
            // No clear, no destructors, etc. needed. Just reset the offset into the buffer.
            m_currOffset = 0;
        }

    private:

        alignas(CacheLineSize) std::byte m_buffer[JobPoolBufferSize];
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
        [[nodiscard]] Job* allocate(uint32_t version) noexcept
        {
            Job* job = nullptr;

            auto blockIndex = m_currentBlock.load(std::memory_order_relaxed);
            auto* block = m_blocks[blockIndex].get();
            auto offset = block->currOffset.fetch_add(sizeof(Job), std::memory_order_relaxed);

            if ((offset + sizeof(Job)) <= JobPoolBufferSize)
            {
                // Room in the current block, so allocate from there.
                job = new (block->buffer + offset) Job{ .version = version };
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

                job = new (block->buffer + 0) Job{ .version = version };
            }

            return job;
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

    private:

        struct Block
        {
            alignas(CacheLineSize) std::byte buffer[JobPoolBufferSize];
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
        Job* job = nullptr;

        if (threadIndex < m_pImpl->localPools.size())
        {
            job = m_pImpl->localPools[threadIndex]->allocate(m_pImpl->version);
        }

        if (job == nullptr)
        {
            job = m_pImpl->globalPool.allocate(m_pImpl->version);
        }

        assert(job != nullptr);

        job->func = func;
        job->data = externalData;

        return { job, job->version };
    }

    bool JobPool::addDependency(JobHandle dependent, JobHandle dependency) const noexcept
    {
        if ((dependent.job == nullptr) || (dependency.job == nullptr) || (dependent.job->version != dependency.job->version))
        {
            return false;
        }

        auto dependentIndex = dependency.job->dependentsCount.fetch_add(1, std::memory_order_relaxed);

        if (dependentIndex >= dependency.job->dependents.size())
        {
            // The dependency is already maxed out in dependents.
            return false;
        }

        dependency.job->dependents[dependentIndex] = dependent;
        dependent.job->dependencyCount.fetch_add(1, std::memory_order_relaxed);

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
}