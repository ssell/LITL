#include <vector>

#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"

namespace LITL::Engine::ECS
{
    Archetype::Archetype()
        : m_chunks(16)      // 16kb chunks * 16 = 256kb pages
    {

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