#ifndef LITL_CORE_CONTAINERS_MEMORY_ARENA_H__
#define LITL_CORE_CONTAINERS_MEMORY_ARENA_H__

#include <cassert>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>

namespace LITL::Core
{
    /// <summary>
    /// Current state of the MemoryArena.
    /// </summary>
    struct MemoryArenaState
    {
        uint32_t blocksAllocated{ 0 };
        uint32_t blocksInUse{ 0 };
        uint32_t bytesAllocated{ 0 };
        uint32_t bytesInUse{ 0 };
        uint32_t currBlockOffset{ 0 };
    };

    /// <summary>
    /// A generic, block-based memory arena.
    /// All memory is "freed" (but not zeroed-out) on call to reset.
    /// </summary>
    /// <typeparam name="BlockSize"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    template<uint32_t BlockSize = 1024>
    class MemoryArena
    {
        /// <summary>
        /// A single block of a self-maintaining chain of memory blocks.
        /// When one block fills up, it creates a new block to insert into.
        /// </summary>
        /// <typeparam name="Size"></typeparam>
        /// <typeparam name="Alignment"></typeparam>
        template<uint32_t Size>
        struct MemoryBlock
        {
            /// <summary>
            /// Attempts to copy the data into the block.
            /// If the block does not have sufficient room to store the data then this returns null.
            /// </summary>
            /// <param name="source"></param>
            /// <param name="size"></param>
            /// <param name="alignment"></param>
            /// <returns></returns>
            [[nodiscard]] void* copyInto(void const* source, uint32_t& offset, uint32_t size, uint32_t alignment) noexcept
            {
                assert(source != nullptr);
                assert((size + alignment) <= Size);

                const uint32_t aligned = (offset + alignment - 1) & ~(alignment - 1);

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

            MemoryBlock<Size>* next{ nullptr };
            alignas(alignof(std::max_align_t)) std::byte buffer[Size];
        };

    public:

        static constexpr uint32_t MemoryBlockSize = BlockSize;

        MemoryArena() 
            : m_pRoot(new(std::nothrow) MemoryBlock<BlockSize>()), m_pCurrent{ m_pRoot }
        {
            assert(m_pRoot != nullptr);

            m_state.blocksAllocated = 1;
            m_state.blocksInUse = 1;
            m_state.bytesAllocated = MemoryBlockSize;
            m_state.bytesInUse = 0;
        }

        MemoryArena(MemoryArena<BlockSize>&& other)
        {
            m_pRoot = other.m_pRoot;
            m_pCurrent = other.m_pCurrent;
            m_state.currBlockOffset = other.m_state.currBlockOffset;
            m_state = other.m_state;

            other.m_pRoot = nullptr;
            other.m_pCurrent = nullptr;
            other.m_state.currBlockOffset = 0;
        }

        MemoryArena<BlockSize>& operator=(MemoryArena<BlockSize>&& other)
        {
            m_pRoot = other.m_pRoot;
            m_pCurrent = other.m_pCurrent;
            m_state.currBlockOffset = other.m_state.currBlockOffset;
            m_state = other.m_state;

            other.m_pRoot = nullptr;
            other.m_pCurrent = nullptr;
            other.m_state.currBlockOffset = 0;

            return (*this);
        }

        ~MemoryArena()
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

        MemoryArena(MemoryArena const&) = delete;
        MemoryArena& operator=(MemoryArena const&) = delete;

        /// <summary>
        /// Copies the provided data into the the memory arena.
        /// The source data must fit completely inside of a single block (see MemoryBlockSize).
        /// May return null if allocation of an additional block is required and fails.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="size"></param>
        /// <param name="alignment"></param>
        /// <returns></returns>
        [[nodiscard]] void* copyInto(void const* source, uint32_t size, uint32_t alignment) noexcept
        {
            assert((alignment > 0) && ((alignment & (alignment - 1)) == 0));    // alignment must be non-zero, power-of-two
            assert((size + alignment - 1) <= BlockSize);                        // aligned object must fit entirely within a single block

            uint32_t prevOffset = m_state.currBlockOffset;
            auto* dest = m_pCurrent->copyInto(source, m_state.currBlockOffset, size, alignment);

            if (dest != nullptr)
            {
                // successful data copy
                m_state.bytesInUse += (m_state.currBlockOffset - prevOffset);
            }
            else
            {
                // failed data copy due to insufficient space.
                auto* next = m_pCurrent->next;

                if (next != nullptr)
                {
                    // we have a pre-existing unused memory block from a previous reset.
                    m_pCurrent = next;
                }
                else
                {
                    // need to allocate a new block of memory.
                    m_pCurrent->next = new(std::nothrow) MemoryBlock<BlockSize>();
                    m_pCurrent = m_pCurrent->next;

                    if (m_pCurrent == nullptr)
                    {
                        // allocation of an additional memory block has failed
                        return m_pCurrent;
                    }

                    m_state.blocksAllocated++;
                }

                m_state.currBlockOffset = 0;

                dest = m_pCurrent->copyInto(source, m_state.currBlockOffset, size, alignment);

                m_state.blocksInUse++;
                m_state.bytesAllocated = (m_state.blocksAllocated * MemoryBlockSize);
                m_state.bytesInUse = ((m_state.blocksInUse - 1) * MemoryBlockSize) + m_state.currBlockOffset;
            }

            return dest;
        }

        /// <summary>
        /// Copies the provided data into the the memory arena.
        /// The source data must fit completely inside of a single block (see MemoryBlockSize).
        /// May return null if allocation of an additional block is required and fails.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source"></param>
        /// <returns></returns>
        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] T* copyInto(T const& source) noexcept
        {
            static_assert((sizeof(T) + alignof(T) - 1) <= BlockSize, "Aligned T exceeds memory arena block size");
            return static_cast<T*>(copyInto(&source, sizeof(T), alignof(T)));
        }

        /// <summary>
        /// A non-shrinking reset of the arena.
        /// When the arena is reset, the internal memory is not zeroed-out, 
        /// but will be overwritten on future calls to copyInto.
        /// </summary>
        void reset() noexcept
        {
            m_pCurrent = m_pRoot;
            m_state.blocksInUse = 1;
            m_state.bytesInUse = 0;
            m_state.currBlockOffset = 0;
        }

        /// <summary>
        /// A shrinking reset of the arena to place it at a size of at most the specified number of blocks.
        /// When the arena is reset, the internal memory is not zeroed-out, 
        /// but will be overwritten on future calls to copyInto.
        /// </summary>
        /// <param name="blockCount"></param>
        void resetShrink(uint32_t blockCount = 1) noexcept
        {
            assert(blockCount > 0);

            MemoryBlock<BlockSize>* curr = m_pRoot;
            uint32_t i = 0;

            for (i = 0; (i < blockCount) && (curr != nullptr); ++i)
            {
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
            m_state.currBlockOffset = 0;
            m_state.blocksInUse = 0;
            m_state.bytesInUse = 0;
            m_state.blocksAllocated = i;
            m_state.bytesAllocated = (MemoryBlockSize * i);
        }

        /// <summary>
        /// Returns information about the current internal state of the arena.
        /// </summary>
        /// <returns></returns>
        MemoryArenaState getState() const noexcept
        {
            return m_state;
        }

    private:

        MemoryBlock<BlockSize>* m_pRoot;
        MemoryBlock<BlockSize>* m_pCurrent;
        MemoryArenaState m_state{};
    };
}

#endif