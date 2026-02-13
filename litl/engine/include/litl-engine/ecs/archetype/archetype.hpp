#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <vector>

#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"

namespace LITL::Engine::ECS
{
    class Archetype
    {
    public:

        ArchetypeKey key() const noexcept;
        ArchetypeColumn* column(ComponentTypeId componentType) noexcept;
        size_t entityCount() const noexcept;
        size_t componentCount() const noexcept;

        template<typename T>
        T* getTypedArray(ArchetypeColumn* column)
        {
            return reinterpret_cast<T*>(column->data());
        }

        static void move(Archetype* from, Archetype* to, Entity entity, size_t entityIndex);

    protected:

    private:

        ArchetypeKey m_key;
        std::vector<Entity> m_entities;
        std::vector<ArchetypeColumn> m_columns;
    };
}

#endif