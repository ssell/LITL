#ifndef LITL_CORE_CONTAINERS_MEMORY_POOL_H__
#define LITL_CORE_CONTAINERS_MEMORY_POOL_H__

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "litl-core/constants.hpp"

namespace LITL::Core
{
    /// <summary>
    /// A single block of a self-maintaining chain of memory blocks.
    /// When one block fills up, it creates a new block to insert into.
    /// </summary>
    /// <typeparam name="Size"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    template<size_t Size, size_t Alignment>
    class MemoryBlock
    {
    public:

        MemoryBlock() = default;

        ~MemoryBlock()
        {
            if (m_pNext != nullptr)
            {
                delete m_pNext;
                m_pNext = nullptr;
            }
        }

        MemoryBlock(MemoryBlock const&) = delete;
        MemoryBlock& operator=(MemoryBlock const&) = delete;

        void* copyInto(void* source, size_t size, MemoryBlock<Size, Alignment>** owningBlock) noexcept
        {
            assert(source != nullptr);
            assert(size <= Size);

            void* dest = nullptr;

            if ((size + m_offset) <= Size)
            {
                dest = get(m_offset);
                memcpy(dest, source, size);
                *owningBlock = this;
                m_offset += size;
            }
            else
            {
                if (m_pNext == nullptr)
                {
                    m_pNext = new MemoryBlock<Size, Alignment>();
                }

                dest = m_pNext->copyInto(source, size, owningBlock);
            }

            assert(dest != nullptr);

            return dest;
        }

        void* get(size_t offset)
        {
            assert(offset < Size);
            return &m_buffer[offset];
        }

        void setNext(MemoryBlock<Size, Alignment>* next)
        {
            assert(next != nullptr);
            assert(m_pNext == nullptr);
            m_pNext = next;
        }

        void reset()
        {
            m_offset = 0;

            if (m_pNext != nullptr)
            {
                m_pNext->reset();
            }
        }

    protected:

    private:

        MemoryBlock<Size, Alignment>* m_pNext{ nullptr };
        alignas(Alignment) std::byte m_buffer[Size];
        size_t m_offset{ 0 };
    };

    /// <summary>
    /// A generic, block-based memory pool.
    /// </summary>
    /// <typeparam name="BlockSize"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    template<size_t BlockSize = 1024, size_t Alignment = Constants::cache_line_size>
    class MemoryPool
    {
    public:

        MemoryPool() : m_pCurrent{ &m_root }
        {

        }

        ~MemoryPool() = default;

        MemoryPool(MemoryPool const&) = delete;
        MemoryPool& operator=(MemoryPool const&) = delete;

        void* copyInto(void* source, size_t size) noexcept
        {
            return m_pCurrent->copyInto(source, size, &m_pCurrent);
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        void extract(void* source, void* destination) noexcept
        {
            memcpy(destination, source, sizeof(T));
        }

        void reset() noexcept
        {
            m_root.reset();
        }

    protected:

    private:

        MemoryBlock<BlockSize, Alignment> m_root{};
        MemoryBlock<BlockSize, Alignment>* m_pCurrent{ nullptr };
    };
}

#endif