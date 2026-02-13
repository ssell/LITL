#include <vector>

#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetype.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"

namespace LITL::Engine::ECS
{
    ArchetypeKey Archetype::key() const noexcept
    {
        return m_key;
    }

    ArchetypeColumn* Archetype::column(ComponentTypeId componentType) noexcept
    {
        // m_columns is sorted by component type
        for (size_t i = 0; i < m_columns.size(); ++i)
        {
            if (m_columns[i].componentType() == componentType)
            {
                return &(m_columns[i]);
            }
        }

        return nullptr;
    }

    size_t Archetype::entityCount() const noexcept
    {
        return m_entities.size();
    }

    size_t Archetype::componentCount() const noexcept
    {
        return m_columns.size();
    }

    void Archetype::move(Archetype* from, Archetype* to, Entity entity, size_t entityIndex)
    {
        // Move the entity (ezpz)
        to->m_entities.push_back(entity);

        // Move the matching components
        for (size_t i = 0; i < from->m_columns.size(); ++i)
        {
            auto* componentDescriptor = from->m_columns[i].descriptor();

            auto* fromCol = &from->m_columns[i];
            auto* toCol = to->column(componentDescriptor->id);  // do not access to->m_columns directly as it may not have this component

            if (toCol != nullptr)
            {
                auto* component = fromCol->get(entityIndex);

                toCol->move(component);
                componentDescriptor->destroy(component);
            }
        }

        // Swap + remove the entity
        from->m_entities[entityIndex] = from->m_entities.back();
        from->m_entities.pop_back();

        for (auto& column : from->m_columns)
        {
            column.remove(entityIndex);
        }
    }
}