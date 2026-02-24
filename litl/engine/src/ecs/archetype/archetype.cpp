#include <algorithm>
#include <optional>
#include <string_view>

#include "litl-core/math/math.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype(std::string_view name, uint32_t registryIndex, uint64_t componentHash) : 
        m_registryIndex(registryIndex),
        m_componentHash(componentHash),
        m_entityCount(0),
        m_chunks(16)                        // 16kb chunks * 16 = 256kb pages
    {
        setDebugName(name);

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

    std::vector<ComponentTypeId> const& Archetype::componentTypes() const noexcept
    {
        return m_components;
    }

    uint32_t Archetype::componentCount() const noexcept
    {
        return static_cast<uint32_t>(m_components.size());
    }

    uint32_t Archetype::entityCount() const noexcept
    {
        return m_entityCount;
    }

    ChunkLayout const& Archetype::chunkLayout() const noexcept
    {
        return m_chunkLayout;
    }

    Chunk& Archetype::getChunk(EntityRecord const record) noexcept
    {
        assert(record.archetype == this);
        assert(record.archetypeIndex < m_entityCount);

        return getChunk(record.archetypeIndex / m_chunkLayout.entityCapacity);
    }

    Chunk& Archetype::getChunk(uint32_t const index) noexcept
    {
        assert(index < m_chunks.size());
        return m_chunks[index];
    }

    uint32_t Archetype::getNextIndex() noexcept
    {
        if (m_entityCount == 0)
        {
            // First entity in this archetype (archetype may be new or used to have entities that have since left)
            if (m_chunks.size() == 0)
            {
                // First EVER in this archetype. Allocate the first chunk.
                m_chunks.emplace_back(m_chunks.size(), &m_chunkLayout);
            }
        }
        else if (m_entityCount % m_chunkLayout.entityCapacity == 0)
        {
            // The last chunk is currently full.
            m_chunks.emplace_back(m_chunks.size(), &m_chunkLayout);
        }

        return m_entityCount++;
    }

    bool Archetype::hasComponent(ComponentTypeId componentTypeId) const noexcept
    {
        size_t unused = 0;
        return hasComponent(componentTypeId, unused);
    }

    bool Archetype::hasComponent(ComponentTypeId componentTypeId, size_t& index) const noexcept
    {
        for (auto i = 0; i < m_chunkLayout.componentTypeCount; ++i)
        {
            if (m_chunkLayout.componentOrder[i]->id == componentTypeId)
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
       const auto chunkIndex = archetypeIndex / m_chunkLayout.entityCapacity;
       const auto chunkElementIndex = archetypeIndex % m_chunkLayout.entityCapacity;

       m_chunks[chunkIndex].add(m_chunkLayout, chunkElementIndex, record.entity);

       EntityRegistry::updateRecordArchetype(record.entity, this, archetypeIndex);
    }

    void Archetype::remove(EntityRecord const& record) noexcept
    {
        if (record.archetype != this || record.archetypeIndex >= m_entityCount)
        {
            return;
        }

        m_entityCount = Math::maximum(m_entityCount - 1, 0u);

        // Get the chunk and element index for where we are removing
        const auto removeFromArchetypeIndex = record.archetypeIndex;
        const auto removeFromChunkIndex = record.archetypeIndex / m_chunkLayout.entityCapacity;
        const auto removeFromChunkElementIndex = record.archetypeIndex % m_chunkLayout.entityCapacity;

        // Get the chunk and element index for the entity we swapping into our newly opened spot.
        const auto swapWithChunkIndex = m_entityCount / m_chunkLayout.entityCapacity;
        const auto swapWithChunkElementIndex = m_entityCount % m_chunkLayout.entityCapacity;

        auto* removeFromChunk = &m_chunks[removeFromChunkIndex];
        auto* swapWithChunk = &m_chunks[swapWithChunkIndex];

        auto swappedEntity = removeFromChunk->removeAndSwap(m_chunkLayout, removeFromChunkElementIndex, swapWithChunk, swapWithChunkElementIndex);

        if (swappedEntity != std::nullopt)
        {
            auto swappedEntityRecord = EntityRegistry::getRecord(swappedEntity.value());
            EntityRegistry::updateRecordArchetypeIndex(swappedEntityRecord.entity, removeFromArchetypeIndex);
        }

        EntityRegistry::updateRecordArchetype(record.entity, nullptr, 0);
    }

    void Archetype::move(EntityRecord const& record, Archetype* to) noexcept
    {
        if ((record.archetype != this) || (to == this))
        {
            return;
        }

        // Get the chunk and element index for where we are removing
        const auto fromArchetypeIndex = record.archetypeIndex;
        const auto fromChunkIndex = fromArchetypeIndex / m_chunkLayout.entityCapacity;
        const auto fromChunkElementIndex = fromArchetypeIndex % m_chunkLayout.entityCapacity;

        // Get the chunk and element index for where we are adding to
        const auto toArchetypeIndex = to->getNextIndex();
        const auto toChunkIndex = toArchetypeIndex / to->m_chunkLayout.entityCapacity;
        const auto toChunkElementIndex = toArchetypeIndex % m_chunkLayout.entityCapacity;

        auto fromChunkData = m_chunks[fromChunkIndex].data();
        auto toChunkData = to->m_chunks[toChunkIndex].data();

        // Move entity
        auto fromChunkEntityAddr = (fromChunkData + m_chunkLayout.entityArrayOffset + (fromChunkElementIndex * sizeof(Entity)));
        auto toChunkEntityAddr = (toChunkData + to->m_chunkLayout.entityArrayOffset + (toChunkElementIndex * sizeof(Entity)));
        new (toChunkEntityAddr) Entity(std::move(*reinterpret_cast<Entity*>(fromChunkEntityAddr)));
        
        // Move components into the new archetype AND instantiate any missing ones
        ComponentDescriptor const* component = nullptr;
        std::byte* componentAddress = nullptr;
        size_t componentIndex = 0;

        for (auto i = 0; i < to->m_chunkLayout.componentTypeCount; ++i)
        {
            // A component in the new archetype.
            component = to->m_chunkLayout.componentOrder[i];
            componentAddress = (toChunkData + to->m_chunkLayout.componentOffsets[i] + (toChunkElementIndex * component->size));
            componentIndex = static_cast<size_t>(0);

            // Does this entity already have this component?
            if (hasComponent(component->id, componentIndex))
            {
                auto fromComponentAddress = (fromChunkData + m_chunkLayout.componentOffsets[componentIndex] + (fromChunkElementIndex * component->size));
                component->move(fromComponentAddress, componentAddress);
            }
            // Otherwise instantiate a new component
            else
            {
                component->build(componentAddress);
            }
        }

        to->m_chunks[toChunkIndex].incrementEntityCount();

        // Destroy any components not making it into the new archetype (make sure the destructors are called)
        for (auto i = 0; i < m_chunkLayout.componentTypeCount; ++i)
        {
            component = m_chunkLayout.componentOrder[i];
            componentAddress = (fromChunkData + m_chunkLayout.componentOffsets[i] + (fromChunkElementIndex * component->size));

            if (!to->hasComponent(component->id, componentIndex))
            {
                component->destroy(componentAddress);
            }
        }

        // Remove the entity from this archetype (this calls m_entityCount--)
        remove(record);

        // Update it's record to point to it's new archetype
        EntityRegistry::updateRecordArchetype(record.entity, to, toArchetypeIndex);
    }
}