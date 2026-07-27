#ifndef LITL_ENGINE_ECS_ENTITY_RECORD_H__
#define LITL_ENGINE_ECS_ENTITY_RECORD_H__

#include <cstdint>
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class Archetype;
    class EntityRegistry;

    /// <summary>
    /// big-ol' todo
    /// 
    /// this maintains the mapping between entity -> archetype + index.
    /// this mapping must be maintained (updated) whenever an entity is
    /// created, destroyed (and swapped), or moved between archetypes.
    /// 
    /// this relationship also should be asserted. and a test or two 
    /// wouldn't hurt, either.
    /// </summary>
    struct EntityRecord
    {
        Entity entity = Entity::null();

        /// <summary>
        /// The archetype to which this Entity matches.
        /// If this is null, then the Entity is currently dead.
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