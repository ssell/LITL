#ifndef LITL_ENGINE_ECS_ARCHETYPE_H__
#define LITL_ENGINE_ECS_ARCHETYPE_H__

#include <vector>

#include "litl-core/containers/flatHashMap.hpp"
#include "litl-engine/ecs/component.hpp"
#include "litl-engine/ecs/entity.hpp"
#include "litl-engine/ecs/archetype/archetypeKey.hpp"
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"

namespace LITL::Engine::ECS
{
    class Archetype
    {
    public:

        ArchetypeKey key;
        std::vector<Entity> entities;
        //Core::FlatHashMap<ArchetypeKey, std::unique_ptr<IArchetypeColumn>> columns;

    protected:

    private:
    };
}

#endif