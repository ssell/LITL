#include <algorithm>
#include <vector>

#include "litl-engine/ecs/archetype/archetype.hpp"

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

    size_t Archetype::entityCount() const noexcept
    {
        return m_entities.size();
    }
}