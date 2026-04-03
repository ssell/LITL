#ifndef LITL_CORE_CONTAINERS_MEMORY_ARENA_H__
#define LITL_CORE_CONTAINERS_MEMORY_ARENA_H__

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

#include "litl-core/containers/ringBuffer.hpp"

namespace LITL::Core
{
    /// <summary>
    /// Current state of the MemoryArena.
    /// </summary>
    struct MemoryArenaState
    {
        size_t blockSize{ 0 };
        size_t blocksAllocated{ 0 };
        size_t blocksInUse{ 0 };
        size_t bytesInUse{ 0 };
        size_t currBlockOffset{ 0 };

        [[nodiscard]] constexpr size_t bytesAllocated() const noexcept
        {
            return blockSize * blocksAllocated;
        }
    };

    /// <summary>
    /// A generic, block-based memory arena.
    /// The primary goal of this arena implementation is performance. As such the following design choices are intentional:
    /// 
    /// (1) When the arena is reset, memory is not zeroed out. The internal offset tracker is simply reset.
    /// (2) Space at the end of a block is not tracked and may be wasted. Once the arena moves onto the
    /// next block it never returns to a previous block (until it is reset of course).
    /// (3) The default reset method does not free blocks. resetShrink and resetShrinkAuto serve this purpose.
    /// 
    /// Any pointers to memory within the arena should be considered invalid after a reset and no longer used.
    /// </summary>
    template<size_t BlockSize = 1024, size_t ShrinkLookBack = 32> requires (BlockSize >= 64) && (ShrinkLookBack > 0)
    class MemoryArena
    {
        /// <summary>
        /// A single block in a chain of memory blocks.
        /// When one block fills up, the arena should move to the next block.
        /// </summary>
        /// <typeparam name="Size"></typeparam>
        /// <typeparam name="Alignment"></typeparam>
        struct MemoryBlock
        {
            /// <summary>
            /// Allocates a block of memory in the memory block.
            /// If the block does not have sufficient room then this returns null.
            /// </summary>
            /// <param name="source"></param>
            /// <param name="size"></param>
            /// <param name="alignment"></param>
            /// <returns></returns>
            [[nodiscard]] void* allocate(size_t& offset, size_t size, size_t alignment) noexcept
            {
                assert((size + (alignment - 1)) <= BlockSize);

                void* dest = &buffer[offset];
                size_t available = BlockSize - offset;

                if (!std::align(alignment, size, dest, available))
                {
                    // insufficient space
                    return nullptr;
                }

                // (BlockSize - available) = aligned offset, + size = move past the new allocation.
                offset = ((BlockSize - available) + size);

                return dest;
            }

            MemoryBlock* next{ nullptr };
            alignas(alignof(std::max_align_t)) std::byte buffer[BlockSize];
        };

    public:

        MemoryArena() 
            : m_pRoot(new(std::nothrow) MemoryBlock()), m_pCurrent{ m_pRoot }
        {
            assert(m_pRoot != nullptr);

            m_state.blocksAllocated = 1;
            m_state.blocksInUse = 1;
            m_state.bytesInUse = 0;
        }

        MemoryArena(MemoryArena<BlockSize>&& other)
            : m_pRoot(other.m_pRoot), m_pCurrent(other.m_pCurrent), m_state(other.m_state)
        {
            other.m_pRoot = nullptr;
            other.m_pCurrent = nullptr;
            other.m_state = {};
        }

        MemoryArena<BlockSize>& operator=(MemoryArena<BlockSize>&& other)
        {
            if (this != &other)
            {
                cleanup();

                m_pRoot = other.m_pRoot;
                m_pCurrent = other.m_pCurrent;
                m_state = other.m_state;

                other.m_pRoot = nullptr;
                other.m_pCurrent = nullptr;
                other.m_state = {};
            }

            return (*this);
        }

        ~MemoryArena()
        {
            cleanup();
        }

        MemoryArena(MemoryArena const&) = delete;
        MemoryArena& operator=(MemoryArena const&) = delete;

        /// <summary>
        /// Allocates a block of memory in the memory arena.
        /// If the arena does not have sufficient room then this returns null.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="size"></param>
        /// <param name="alignment"></param>
        /// <returns></returns>
        [[nodiscard]] void* allocate(size_t size, size_t alignment) noexcept
        {
            assert((alignment > 0) && ((alignment & (alignment - 1)) == 0));    // alignment must be non-zero, power-of-two

            size_t prevOffset = m_state.currBlockOffset;
            void* dest = m_pCurrent->allocate(m_state.currBlockOffset, size, alignment);

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
                    m_pCurrent->next = new(std::nothrow) MemoryBlock();
                    m_pCurrent = m_pCurrent->next;

                    if (m_pCurrent == nullptr)
                    {
                        // allocation of an additional memory block has failed
                        return m_pCurrent;
                    }

                    m_state.blocksAllocated++;
                }

                m_state.currBlockOffset = 0;

                dest = m_pCurrent->allocate(m_state.currBlockOffset, size, alignment);

                m_state.blocksInUse++;
                m_state.bytesInUse = ((m_state.blocksInUse - 1) * BlockSize) + m_state.currBlockOffset;
            }

            return dest;
        }

        /// <summary>
        /// Copies the provided data into the the memory arena.
        /// The source data must fit completely inside of a single block (see BlockSize).
        /// May return null if allocation of an additional block is required and fails.
        /// </summary>
        /// <param name="source"></param>
        /// <param name="size"></param>
        /// <param name="alignment"></param>
        /// <returns></returns>
        [[nodiscard]] void* insert(void const* source, size_t size, size_t alignment) noexcept
        {
            assert(source != nullptr);

            auto* dest = allocate(size, alignment);

            if (dest != nullptr)
            {
                memcpy(dest, source, size);
            }

            return dest;
        }

        /// <summary>
        /// Copies the provided data into the the memory arena.
        /// The source data must fit completely inside of a single block.
        /// May return null if allocation of an additional block is required and fails.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="source"></param>
        /// <returns></returns>
        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] T* insert(T const& source) noexcept
        {
            static_assert((sizeof(T) + alignof(T) - 1) <= BlockSize, "Aligned T exceeds memory arena block size");
            return static_cast<T*>(insert(&source, sizeof(T), alignof(T)));
        }

        /// <summary>
        /// Inserts a new object in place into the memory arena.
        /// May return null if the arena is full and failed to allocate an additional block
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <typeparam name="...Args"></typeparam>
        /// <param name="...args"></param>
        /// <returns></returns>
        template<typename T, typename... Args> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] T* emplace(Args&&... args) noexcept
        {
            static_assert((sizeof(T) + alignof(T) - 1) <= BlockSize, "Aligned T exceeds memory arena block size");
            void* dest = allocate(sizeof(T), alignof(T));

            if (dest != nullptr)
            {
                return new(dest) T(std::forward<Args>(args)...);
            }
            else
            {
                return nullptr;
            }
        }

        /// <summary>
        /// A non-shrinking reset of the arena.
        /// When the arena is reset, the internal memory is not zeroed-out, but will be overwritten on future calls to copyInto.
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
        /// When the arena is reset, the internal memory is not zeroed-out, but will be overwritten on future calls to copyInto.
        /// 
        /// Specified block count must be at least 1.
        /// </summary>
        /// <param name="blockCount"></param>
        void resetShrink(size_t blockCount) noexcept
        {
            assert(blockCount > 0);

            MemoryBlock* prev = nullptr;
            MemoryBlock* curr = m_pRoot;
            size_t i = 0;

            // walk until either: (A) we reach the end of the chain or (B) we traversed the specified number of blocks.
            for (; (i < blockCount) && (curr != nullptr); ++i)
            {
                prev = curr;
                curr = curr->next;
            }

            // cut off the end of the node chain
            if (prev)
            {
                prev->next = nullptr;
            }

            // reset all cut off blocks (if any)
            while (curr != nullptr)
            {
                auto* next = curr->next;
                delete curr;
                curr = next;
            }

            // update internal state
            m_pCurrent = m_pRoot;
            m_state.currBlockOffset = 0;
            m_state.blocksInUse = 1;
            m_state.bytesInUse = 0;
            m_state.blocksAllocated = i;
        }

        /// <summary>
        /// A shrinking reset, that shrinks back to the recent maximum block usage highwater mark.
        /// This automatic shrink looks back upto the previous ShrinkLookBack resets.
        /// For a block count to be considered, this reset must be used.
        /// </summary>
        void resetShrinkAuto() noexcept
        {
            m_prevBlockUsage.add(m_state.blocksInUse);
            resetShrink(m_prevBlockUsage.max());
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

        /// <summary>
        /// Performs a full cleanup.
        /// Used either in destructor or move assignment.
        /// </summary>
        void cleanup()
        {
            if (m_pRoot == nullptr)
            {
                return;
            }

            MemoryBlock* curr = m_pRoot;
            MemoryBlock* next = nullptr;

            while (curr != nullptr)
            {
                next = curr->next;
                delete curr;
                curr = next;
            }

            m_pRoot = nullptr;
            m_pCurrent = nullptr;
        }

        MemoryBlock* m_pRoot;
        MemoryBlock* m_pCurrent;
        MemoryArenaState m_state{ BlockSize };
        RingBuffer<size_t, ShrinkLookBack> m_prevBlockUsage;
    };
}

#endif