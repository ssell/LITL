#ifndef LITL_ENGINE_ECS_ENTITY_RECORD_H__
#define LITL_ENGINE_ECS_ENTITY_RECORD_H__

#include "litl-engine/ecs/archetype.hpp"

namespace LITL::Engine::ECS
{
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
        /// <summary>
        /// The archetype to which this Entity matches.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The index into the Archetype.
        /// </summary>
        size_t index;
    };
}

#endif