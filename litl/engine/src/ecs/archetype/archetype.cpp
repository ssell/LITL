#include <algorithm>
#include <optional>
#include <vector>

#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/entityRegistry.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype(uint32_t registryIndex, uint64_t componentHash) : 
        m_registryIndex(registryIndex),
        m_componentHash(componentHash),
        m_chunks(16)      // 16kb chunks * 16 = 256kb pages
    {
        m_chunkLayout.archetype = this;
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

    void Archetype::remove(EntityRecord* record) noexcept
    {
        if (record->archetype != this || record->archetypeIndex >= m_entityCount)
        {
            return;
        }

        // Get the chunk and element index for where we are removing
        const auto removeFromArchetypeIndex = record->archetypeIndex;
        const auto removeFromChunkIndex = record->archetypeIndex / m_chunkLayout.chunkElementCapacity;
        const auto removeFromChunkElementIndex = record->archetypeIndex % m_chunkLayout.chunkElementCapacity;

        // Get the chunk and element index for the entity we swapping into our newly opened spot.
        const auto swapWithChunkIndex = m_chunks.size() - 1;
        const auto swapWithChunkElementIndex = m_chunks[swapWithChunkIndex].getHeader()->count - 1;

        auto* removeFromChunk = &m_chunks[removeFromChunkIndex];
        auto* swapWithChunk = &m_chunks[swapWithChunkIndex];

        auto swappedEntity = removeFromChunk->removeAndSwap(m_chunkLayout, removeFromChunkElementIndex, swapWithChunk, swapWithChunkElementIndex);

        if (swappedEntity != std::nullopt)
        {
            auto& swappedEntityRecord = EntityRegistry::getRecord(swappedEntity.value());
            swappedEntityRecord.archetypeIndex = removeFromArchetypeIndex;
        }

        record->archetype = nullptr;
        record->archetypeIndex = 0;
    }
}