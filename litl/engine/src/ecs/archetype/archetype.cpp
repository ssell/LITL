#include <algorithm>
#include <vector>

#include "litl-engine/ecs/archetype/archetype.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype(uint32_t key, uint32_t stableKey)
        : m_key(key), m_stableKey(stableKey), m_chunks(16)      // 16kb chunks * 16 = 256kb pages
    {
        m_chunkLayout.archetype = this;
    }

    uint32_t Archetype::key() const noexcept
    {
        return m_key;
    }

    uint32_t Archetype::stableKey() const noexcept
    {
        return m_stableKey;
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