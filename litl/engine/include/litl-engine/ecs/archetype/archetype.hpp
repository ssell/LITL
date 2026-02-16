#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <vector>

#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"
#include "litl-engine/ecs/archetype/chunk.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// An Archetype is an ordered component set.
    /// All entities fit into exactly one Archetype.
    /// </summary>
    class Archetype
    {
    public:

        ArchetypeKey key() const noexcept;
        ArchetypeColumn* column(ComponentTypeId componentType) noexcept;
        size_t entityCount() const noexcept;
        size_t componentCount() const noexcept;

        static void move(Archetype* from, Archetype* to, Entity entity, size_t entityIndex);

    protected:

    private:

        ArchetypeKey m_key;
        std::vector<Entity> m_entities;
        std::vector<ArchetypeColumn> m_columns;
    };
}

#endif