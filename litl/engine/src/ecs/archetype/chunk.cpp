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
        header->capacity = layout->entityCapacity;
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

    void Chunk::add(ChunkLayout const& layout, uint32_t addAtIndex) noexcept
    {
        auto header = getHeader();
        auto to = data();

        // ? construct_at or memcpy here ?
        std::construct_at(to + layout.entityArrayOffset + (sizeof(Entity) * addAtIndex));

        for (auto i = 0; i < layout.componentTypeCount; ++i)
        {
            const auto component = layout.componentOrder[i];
            component->build(to + layout.componentOffsets[i] + (component->size * addAtIndex));
        }

        header->count++;
    }

    std::optional<Entity> Chunk::removeAndSwap(ChunkLayout const& layout, uint32_t const removeAtIndex, Chunk* swapFromChunk, uint32_t const swapFromChunkIndex) noexcept
    {
        auto header = getHeader();

        memset(m_data + layout.entityArrayOffset + (sizeof(Entity) * removeAtIndex), 0, sizeof(Entity));
        header->count--;

        // Move the entity in from the other chunk
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
                    to + layout.entityArrayOffset + (sizeof(Entity) * removeAtIndex),
                    from + layout.entityArrayOffset + (sizeof(Entity) * swapFromChunkIndex),
                    sizeof(Entity));

                // Components
                for (auto i = 0; i < layout.componentTypeCount; ++i)
                {
                    const auto component = layout.componentOrder[i];

                    component->move(
                        from + layout.componentOffsets[i] + (component->size * swapFromChunkIndex),
                        to + layout.componentOffsets[i] + (component->size * removeAtIndex));
                }

                header->count++;
                
                // Finally remove the swapped entity from the other chunk
                return swapFromChunk->removeAndSwap(layout, swapFromChunkIndex, nullptr, 0);
            }
        }
        else
        {
            return std::nullopt;
        }
    }
}