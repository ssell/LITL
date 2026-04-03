#ifndef LITL_CORE_CONTAINERS_MEMORY_POOL_H__
#define LITL_CORE_CONTAINERS_MEMORY_POOL_H__

#include <cassert>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

namespace LITL::Core
{
    /// <summary>
    /// A generic, block-based memory pool.
    /// </summary>
    /// <typeparam name="BlockSize"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    template<size_t BlockSize = 1024>
    class MemoryPool
    {
        /// <summary>
        /// A single block of a self-maintaining chain of memory blocks.
        /// When one block fills up, it creates a new block to insert into.
        /// </summary>
        /// <typeparam name="Size"></typeparam>
        /// <typeparam name="Alignment"></typeparam>
        template<size_t Size>
        struct MemoryBlock
        {
            [[nodiscard]] void* copyInto(void const* source, size_t size, size_t alignment) noexcept
            {
                assert(source != nullptr);
                assert((size + alignment) <= Size);

                const size_t aligned = (offset + alignment - 1) & ~(alignment - 1);

                if ((aligned + size) > Size)
                {
                    // block is full
                    return nullptr;
                }

                void* dest = &buffer[aligned];
                memcpy(dest, source, size);
                offset = aligned + size;

                return dest;
            }

            void reset()
            {
                // intentionally only reset the offset, and not the buffer contents themselves.
                offset = 0;
            }

            MemoryBlock<Size>* next{ nullptr };
            size_t offset{ 0 };
            alignas(alignof(std::max_align_t)) std::byte buffer[Size];
        };

        MemoryBlock<BlockSize>* m_pRoot;
        MemoryBlock<BlockSize>* m_pCurrent;

    public:

        static constexpr size_t MemoryBlockSize = BlockSize;

        MemoryPool() : m_pRoot(new(std::nothrow) MemoryBlock<BlockSize>()), m_pCurrent{ m_pRoot }
        {
            assert(m_pRoot != nullptr);
        }

        MemoryPool(MemoryPool<BlockSize>&& other)
        {
            m_pRoot = other.m_pRoot;
            m_pCurrent = other.m_pCurrent;

            other.m_pRoot = nullptr;
            other.m_pCurrent = nullptr;
        }

        MemoryPool<BlockSize>& operator=(MemoryPool<BlockSize>&& other)
        {
            m_pRoot = other.m_pRoot;
            m_pCurrent = other.m_pCurrent;

            other.m_pRoot = nullptr;
            other.m_pCurrent = nullptr;

            return (*this);
        }

        ~MemoryPool()
        {
            MemoryBlock<BlockSize>* curr = m_pRoot;
            MemoryBlock<BlockSize>* next = nullptr;

            while (curr != nullptr)
            {
                next = curr->next;
                delete curr;
                curr = next;
            }
        }

        MemoryPool(MemoryPool const&) = delete;
        MemoryPool& operator=(MemoryPool const&) = delete;

        [[nodiscard]] void* copyInto(void const* source, size_t size, size_t alignment) noexcept
        {
            assert(size + alignment - 1 <= BlockSize);
            assert(alignment > 0 && (alignment & (alignment - 1)) == 0);        // alignment must be non-zero, power-of-two

            auto* dest = m_pCurrent->copyInto(source, size, alignment);

            if (dest == nullptr)
            {
                auto* next = m_pCurrent->next;

                if (next == nullptr)
                {
                    m_pCurrent->next = new(std::nothrow) MemoryBlock<BlockSize>();
                    m_pCurrent = m_pCurrent->next;

                    assert(m_pCurrent != nullptr);
                }
                else
                {
                    m_pCurrent = next;
                }

                dest = m_pCurrent->copyInto(source, size, alignment);
            }

            assert(dest != nullptr);

            return dest;
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] T* copyInto(T const& source) noexcept
        {
            static_assert(sizeof(T) + alignof(T) - 1 <= BlockSize, "T exceeds memory pool block size");
            return static_cast<T*>(copyInto(&source, sizeof(T), alignof(T)));
        }

        /// <summary>
        /// A non-shrinking reset of the pool.
        /// When the pool is reset, the internal memory is not zeroed-out, 
        /// but will be overwritten on future calls to copyInto.
        /// </summary>
        void reset() noexcept
        {
            auto* block = m_pRoot;

            while (block != nullptr)
            {
                block->reset();
                block = block->next;
            }

            m_pCurrent = m_pRoot;
        }

        /// <summary>
        /// A shrinking reset of the pool to place it at a size of at most the specified number of blocks.
        /// When the pool is reset, the internal memory is not zeroed-out, 
        /// but will be overwritten on future calls to copyInto.
        /// </summary>
        /// <param name="blockCount"></param>
        void resetShrink(size_t blockCount = 1) noexcept
        {
            assert(blockCount > 0);

            MemoryBlock<BlockSize>* curr = m_pRoot;
            size_t i = 0;

            for (i = 0; (i < blockCount) && (curr != nullptr); ++i)
            {
                curr->reset();
                curr = curr->next;
            }

            if (curr != nullptr)
            {
                MemoryBlock<BlockSize>* prev = curr;
                curr = curr->next;
                prev->next = nullptr;

                while (curr != nullptr)
                {
                    auto* next = curr->next;
                    delete curr;
                    curr = next;
                }
            }

            m_pCurrent = m_pRoot;
        }
    };
}

#endif