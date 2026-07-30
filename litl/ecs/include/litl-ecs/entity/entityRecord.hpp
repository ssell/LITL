#ifndef LITL_ENGINE_ECS_ENTITY_RECORD_H__
#define LITL_ENGINE_ECS_ENTITY_RECORD_H__

#include <cstdint>
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class Archetype;
    class EntityRegistry;

    struct EntityRecord
    {
        Entity entity = Entity::null();

        /// <summary>
        /// The archetype to which this Entity matches.
        /// If this is null, then the Entity has never been created.
        /// </summary>
        Archetype* archetype = nullptr;

        /// <summary>
        /// The id of the archetype. Used for one less pointer redirection.
        /// </summary>
        ArchetypeId archetypeId = ecs::Constants::null_archetype_id;

        /// <summary>
        /// The index into the Archetype.
        /// </summary>
        uint32_t archetypeIndex = 0u;

    private:

        friend class EntityRegistry;

        void update(Archetype* pArchetype, uint32_t index) noexcept;
    };
}

#endif