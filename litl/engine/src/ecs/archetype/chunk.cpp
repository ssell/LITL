#include <new>

#include"litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    Chunk::Chunk(uint32_t const index, ChunkLayout const* layout)
    {
        new (m_data) ChunkHeader();
        auto header = getHeader();
        header->archetype = layout->archetype;
        header->count = 0;
        header->capacity = layout->chunkElementCapacity;
        header->chunkIndex = index;
        header->version = 0;
    }

    Chunk::~Chunk()
    {

    }

    ChunkHeader* Chunk::getHeader() noexcept
    {
        return reinterpret_cast<ChunkHeader*>(m_data + 0);
    }

    ChunkEntities* Chunk::getEntities() noexcept
    {
        return reinterpret_cast<ChunkEntities*>(m_data + sizeof(ChunkHeader));
    }

    std::byte* Chunk::data() noexcept
    {
        return m_data;
    }

    std::optional<Entity> Chunk::removeAndSwap(ChunkLayout const& layout, uint32_t const removeAtIndex, Chunk* swapFromChunk, uint32_t const swapFromChunkIndex) noexcept
    {
        auto header = getHeader();

        // First remove the entity and its components from this chunk
        
        /**
         * Clearing the memory here likely isn't even needed, and may be detrimental to maximum performance.
         * All deletions are remove+swaps, so we really only need the swap. Iteration through the chunk
         * is based on the number of entities stored within it, so "dirty" memory regions that have been
         * deleted are never even touched as long as the iteration itself is correct AND deletion itself
         * is done during ECS sync points.
         * 
         * The deletion code is left in place to punctuate the fact that it is knowingly excluded.
         * And to make it easier to put back in incase it is needed :)
         */

        /*
        memset(m_data + layout.entityArrayOffset + (sizeof(Entity) * removeAtIndex), 0, sizeof(Entity));

        for (auto i = 0; i < layout.componentTypeCount; ++i)
        {
            const auto componentSize = layout.componentOrder[i]->size;
            memset(m_data + layout.componentOffsets[i] + (componentSize * removeAtIndex), 0, componentSize);
        }
        */

        header->count--;

        // Next move the entity in from the other chunk
        if (swapFromChunk != nullptr)
        {
            if ((swapFromChunk == this) && (swapFromChunkIndex == removeAtIndex))
            {
                // We were removing the very last/back entity in the archetype. So nothing to swap.
                return std::nullopt;
            }
            else
            {
                // Swap in the Entity and it's components
                auto from = swapFromChunk->data();
                auto to = data();

                // Entity
                memcpy(
                    to + layout.entityArrayOffset + (sizeof(Entity) + removeAtIndex),
                    from + layout.entityArrayOffset + (sizeof(Entity) * swapFromChunkIndex),
                    sizeof(Entity));

                // Components
                for (auto i = 0; i < layout.componentTypeCount; ++i)
                {
                    const auto componentSize = layout.componentOrder[i]->size;

                    memcpy(
                        to + layout.componentOffsets[i] + (componentSize * removeAtIndex),
                        from + layout.componentOffsets[i] + (componentSize * swapFromChunkIndex),
                        componentSize);
                }

                header->count++;
                
                // Finally remove the swapped entity from the other chunk
                swapFromChunk->removeAndSwap(layout, swapFromChunkIndex, nullptr, 0);
            }
        }
        else
        {
            return std::nullopt;
        }
    }
}