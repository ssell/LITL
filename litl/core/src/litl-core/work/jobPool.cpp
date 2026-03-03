#include <atomic>
#include <vector>

#include "litl-core/alignment.hpp"
#include "litl-core/work/jobPool.hpp"

namespace LITL::Core
{
    /// <summary>
    /// Each job pool (both thread-specific and general pool blocks) have an internal buffer of 32KB.
    /// </summary>
    static constexpr uint32_t JobPoolBufferSize = 32 * 1024;

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

        [[nodiscard]] Job* allocate(uint32_t version) noexcept
        {
            Job* job = nullptr;

            auto* block = m_blocks[m_currentBlock.load(std::memory_order_relaxed)].get();
            auto offset = block->currOffset.fetch_add(sizeof(Job), std::memory_order_relaxed);

            if ((offset + sizeof(Job)) <= JobPoolBufferSize)
            {
                // Room in the current block, so allocate from there.
            }
            else
            {
                // Block is out of room. Move to next block.
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

    };

    JobPool::JobPool(uint32_t capacity)
    {

    }

    JobPool::~JobPool()
    {

    }

    Job* JobPool::createJob(JobFunc func, void* userData) noexcept
    {
        return nullptr;
    }

    Job* JobPool::allocate() noexcept
    {
        return nullptr;
    }

    void JobPool::addDependency(Job* job, Job* dependsOn) noexcept
    {

    }
}