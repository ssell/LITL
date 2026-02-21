#include <algorithm>
#include <optional>

#include "litl-core/math/math.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeRegistry.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype(uint32_t registryIndex, uint64_t componentHash) : 
        m_registryIndex(registryIndex),
        m_componentHash(componentHash),
        m_entityCount(0),
        m_chunks(16)                        // 16kb chunks * 16 = 256kb pages
    {
        m_chunkLayout.archetype = this;
        m_components.reserve(MAX_COMPONENTS);
    }

    uint32_t Archetype::registryIndex() const noexcept
    {
        return m_registryIndex;
    }

    uint64_t Archetype::componentHash() const noexcept
    {
        return m_componentHash;
    }

    ChunkLayout const* Archetype::layout() const noexcept
    {
        return &m_chunkLayout;
    }

    std::vector<ComponentTypeId> const& Archetype::componentTypes() const noexcept
    {
        return m_components;
    }

    uint32_t Archetype::componentCount() const noexcept
    {
        return static_cast<uint32_t>(m_components.size());
    }

    uint32_t Archetype::getNextIndex() noexcept
    {
        if (((m_entityCount == 0) && (m_chunks.size() == 0)) ||         // First entity in this archetype. Allocate the first chunk
            (m_entityCount % m_chunkLayout.chunkElementCapacity == 0))  // The last chunk is currently full.
        {
            // to do this is broken booooiii
          //  m_chunks.push_back(Chunk(m_chunks.size(), &m_chunkLayout));
        }

        return m_entityCount + 1;
    }

    bool Archetype::hasComponent(ComponentTypeId component, size_t& index)
    {
        for (auto i = 0; i < m_chunkLayout.componentTypeCount; ++i)
        {
            if (m_chunkLayout.componentOrder[i]->id == component)
            {
                index = i;
                return true;
            }
        }

        return false;
    }

    void Archetype::add(EntityRecord const& record) noexcept
    {
       const auto archetypeIndex = getNextIndex();
       const auto chunkIndex = archetypeIndex / m_chunkLayout.chunkElementCapacity;
       const auto chunkElementIndex = archetypeIndex % m_chunkLayout.chunkElementCapacity;

       m_chunks[chunkIndex].add(m_chunkLayout, chunkElementIndex);

       EntityRegistry::updateRecordArchetype(record.entity, this, archetypeIndex);
    }

    void Archetype::remove(EntityRecord const& record) noexcept
    {
        if (record.archetype != this || record.archetypeIndex >= m_entityCount)
        {
            return;
        }

        // Get the chunk and element index for where we are removing
        const auto removeFromArchetypeIndex = record.archetypeIndex;
        const auto removeFromChunkIndex = record.archetypeIndex / m_chunkLayout.chunkElementCapacity;
        const auto removeFromChunkElementIndex = record.archetypeIndex % m_chunkLayout.chunkElementCapacity;

        // Get the chunk and element index for the entity we swapping into our newly opened spot.
        const auto swapWithChunkIndex = m_chunks.size() - 1;
        const auto swapWithChunkElementIndex = m_chunks[swapWithChunkIndex].getHeader()->count - 1;

        auto* removeFromChunk = &m_chunks[removeFromChunkIndex];
        auto* swapWithChunk = &m_chunks[swapWithChunkIndex];

        auto swappedEntity = removeFromChunk->removeAndSwap(m_chunkLayout, removeFromChunkElementIndex, swapWithChunk, swapWithChunkElementIndex);

        if (swappedEntity != std::nullopt)
        {
            auto swappedEntityRecord = EntityRegistry::getRecord(swappedEntity.value());
            EntityRegistry::updateRecordArchetypeIndex(swappedEntityRecord.entity, removeFromArchetypeIndex);
        }

        EntityRegistry::updateRecordArchetype(record.entity, nullptr, 0);
        m_entityCount = Math::maximum(m_entityCount - 1, 0u);
    }

    void Archetype::move(EntityRecord const& record, Archetype* to) noexcept
    {
        if ((record.archetype != this) || (to == this))
        {
            return;
        }

        // Get the chunk and element index for where we are removing
        const auto fromArchetypeIndex = record.archetypeIndex;
        const auto fromChunkIndex = fromArchetypeIndex / m_chunkLayout.chunkElementCapacity;
        const auto fromChunkElementIndex = fromArchetypeIndex % m_chunkLayout.chunkElementCapacity;

        // Get the chunk and element index for where we are adding to
        const auto toArchetypeIndex = to->getNextIndex();
        const auto toChunkIndex = toArchetypeIndex / to->m_chunkLayout.chunkElementCapacity;
        const auto toChunkElementIndex = toArchetypeIndex % m_chunkLayout.chunkElementCapacity;

        auto fromChunkData = m_chunks[fromChunkIndex].data();
        auto toChunkData = to->m_chunks[toChunkIndex].data();

        // Move entity
        auto fromChunkEntityAddr = (toChunkData + to->m_chunkLayout.entityArrayOffset + (toChunkElementIndex * sizeof(Entity)));
        auto toChunkEntityAddr = (toChunkData + to->m_chunkLayout.entityArrayOffset + (toChunkElementIndex * sizeof(Entity)));
        new (toChunkEntityAddr) Entity(std::move(*reinterpret_cast<Entity*>(fromChunkEntityAddr)));
        
        // Move components
        for (auto i = 0; i < to->m_chunkLayout.componentTypeCount; ++i)
        {
            // A component in the new archetype.
            auto toComponent = to->m_chunkLayout.componentOrder[i];
            auto toComponentAddress = (toChunkData + to->m_chunkLayout.componentOffsets[i] + (toChunkElementIndex * toComponent->size));
            auto fromComponentIndex = static_cast<size_t>(0);

            // Does this entity already have this component?
            if (hasComponent(toComponent->id, fromComponentIndex))
            {
                auto fromComponentAddress = (fromChunkData + m_chunkLayout.componentOffsets[fromComponentIndex] + (fromChunkElementIndex * toComponent->size));
                toComponent->move(fromComponentAddress, toComponentAddress);
            }
            // Otherwise instantiate a new component
            else
            {
                toComponent->build(toComponentAddress);
            }
        }
        
        to->m_entityCount++;

        // Remove the entity from this archetype
        remove(record);

        // Update it's record to point to it's new archetype
        EntityRegistry::updateRecordArchetype(record.entity, to, toArchetypeIndex);
    }
}