#include <new>

#include"litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/chunk.hpp"

namespace litl
{
    Chunk::Chunk(uint32_t const index, ChunkLayout const* layout)
    {
        new (m_data) ChunkHeader();
        auto* header = getHeader();
        header->archetype = layout->archetype;
        header->count = 0u;
        header->capacity = layout->entityCapacity;
        header->chunkIndex = index;
        header->version = 0u;
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

    std::span<Entity const> Chunk::getEntities(ChunkLayout const& layout) const noexcept
    {
        const uint32_t entityCount = static_cast<uint32_t>(getHeader()->count);

        if (entityCount == 0u)
        {
            return {};
        }

        auto const* ptr = std::launder(reinterpret_cast<Entity const*>(m_data + layout.entityArrayOffset));

        return { ptr, entityCount };
    }

    Entity* Chunk::getEntityPtr(ChunkLayout const& layout) noexcept
    {
        return std::launder(reinterpret_cast<Entity*>(m_data + layout.entityArrayOffset));
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
        getEntityPtr(layout)[addAtIndex] = entity;
        for (auto i = 0; i < layout.componentTypeCount; ++i)
        {
            const auto component = layout.componentOrder[i];
            component->build(to + layout.componentOffsets[i] + (component->size * addAtIndex));
        }

        incrementEntityCount();
    }

    std::optional<Entity> Chunk::removeAndSwap(ChunkLayout const& layout, uint32_t const removeAtIndex, Chunk* swapFromChunk, uint32_t const swapFromChunkIndex) noexcept
    {
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
                auto* from = swapFromChunk->data();
                auto* to = data();

                // Entity swap
                getEntityPtr(layout)[removeAtIndex] = swapFromChunk->getEntityPtr(layout)[swapFromChunkIndex];

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
                return getEntityPtr(layout)[removeAtIndex];
            }
        }
        else
        {
            return std::nullopt;
        }
    }

    std::byte const* Chunk::getComponentArray(ChunkLayout const& layout, ComponentTypeId componentTypeId) const
    {
        uint32_t componentIndex = layout.getComponentIndex(componentTypeId);
        assert(componentIndex < ecs::Constants::max_components);

        return &(m_data[layout.componentOffsets[componentIndex]]);
    }

    void Chunk::setComponentValue(ChunkLayout const& layout, ComponentDescriptor const* component, uint32_t entityChunkIndex, void* from) noexcept
    {
        uint32_t componentIndex = layout.getComponentIndex(component->id);
        assert(componentIndex < ecs::Constants::max_components);

        auto to = data();
        component->move(from, to + layout.componentOffsets[componentIndex] + (component->size * entityChunkIndex));
    }
}