#include <vector>
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype()
        : m_chunks(16)      // 16kb chunks * 16 = 256kb pages
    {
        m_chunkLayout.archetype = this;
    }

    ChunkLayout const* Archetype::layout() const noexcept
    {
        return &m_chunkLayout;
    }

    ArchetypeKey Archetype::key() const noexcept
    {
        return m_key;
    }

    size_t Archetype::entityCount() const noexcept
    {
        return m_entities.size();
    }
}