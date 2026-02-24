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

    ChunkHeader const* Chunk::getHeader() const noexcept
    {
        return reinterpret_cast<ChunkHeader const*>(m_data + 0);
    }

    Entity* Chunk::getEntities(ChunkLayout const& layout) noexcept
    {
        return std::bit_cast<Entity*>(m_data + layout.entityArrayOffset);
    }

    uint32_t Chunk::size() const noexcept
    {
        return getHeader()->count;
    }

    std::byte* Chunk::data() noexcept
    {
        return m_data;
    }


    void Chunk::incrementEntityCount() noexcept
    {
        getHeader()->count++;
    }

    void Chunk::decrementEntityCount() noexcept
    {
        getHeader()->count--;
    }

    void Chunk::add(ChunkLayout const& layout, uint32_t addAtIndex, Entity entity) noexcept
    {
        auto to = data();

        //getEntities(layout)->entities[addAtIndex] = entity;
        getEntities(layout)[addAtIndex] = entity;
        for (auto i = 0; i < layout.componentTypeCount; ++i)
        {
            const auto component = layout.componentOrder[i];
            component->build(to + layout.componentOffsets[i] + (component->size * addAtIndex));
        }

        incrementEntityCount();
    }

    std::optional<Entity> Chunk::removeAndSwap(ChunkLayout const& layout, uint32_t const removeAtIndex, Chunk* swapFromChunk, uint32_t const swapFromChunkIndex) noexcept
    {
        auto entityGoingByeBye = getEntities(layout)[removeAtIndex];
        //memset(m_data + layout.entityArrayOffset + (sizeof(Entity) * removeAtIndex), 0, sizeof(Entity));
        decrementEntityCount();

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
                auto from = swapFromChunk->data();
                auto to = data();

                // Entity swap
                getEntities(layout)[removeAtIndex] = swapFromChunk->getEntities(layout)[swapFromChunkIndex];

                // Components swap
                for (auto i = 0; i < layout.componentTypeCount; ++i)
                {
                    const auto component = layout.componentOrder[i];

                    component->move(
                        from + layout.componentOffsets[i] + (component->size * swapFromChunkIndex),
                        to + layout.componentOffsets[i] + (component->size * removeAtIndex));
                }

                incrementEntityCount();
                
                // Finally remove the swapped entity from the other chunk
                swapFromChunk->removeAndSwap(layout, swapFromChunkIndex, nullptr, 0);

                // Return the entity that was swapped into the removed index
                return getEntities(layout)[removeAtIndex];
            }
        }
        else
        {
            return std::nullopt;
        }
    }

    std::byte const* Chunk::getComponentArray(ChunkLayout const& layout, ComponentTypeId componentTypeId) const
    {
        uint32_t componentIndex = 0;
        layout.getComponentIndex(componentTypeId, componentIndex);
        return &(m_data[layout.componentOffsets[componentIndex]]);
    }
}