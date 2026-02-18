#ifndef LITL_ENGINE_ECS_ARCHETYPE_CHUNK_HEADER_H__
#define LITL_ENGINE_ECS_ARCHETYPE_CHUNK_HEADER_H__

#include <cstdint>
#include "litl-engine/ecs/entity.hpp"

namespace LITL::Engine::ECS
{
    class Archetype;

    /// <summary>
    /// Every Chunk buffer starts with this.
    /// </summary>
    struct ChunkHeader
    {
        /// <summary>
        /// Pointer back to the owning Archetype.
        /// </summary>
        Archetype* archetype;

        /// <summary>
        /// The current number of entities whose components are stored within this Chunk.
        /// This naturally also corresponds to the current number of each component.
        /// </summary>
        uint16_t count;

        /// <summary>
        /// The maximum number of entities whose components could be stored within this Chunk.
        /// This naturally also corresponds to the maximum number of each component.
        /// </summary>
        uint16_t capacity;

        /// <summary>
        /// The index of this Chunk with the owning Archetype.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        uint32_t chunkIndex;

        /// <summary>
        /// Last version in which a component within this Chunk was modified.
        /// The ECS world maintains a current version for the entire system.
        /// </summary>
        uint32_t version;

        // ... todo in the future when adding shared components ...
    };

    /// <summary>
    /// An array of all of the entities in a Chunk.
    /// The capacity and current number of entities are stored in the ChunkHeader.
    /// </summary>
    struct ChunkEntities
    {
        Entity entities[MAX_ENTITIES_PER_CHUNK];
    };
}

#endif