#ifndef LITL_ENGINE_ECS_ENTITY_H__
#define LITL_ENGINE_ECS_ENTITY_H__

#include <cstdint>

namespace LITL::ECS
{
    /// <summary>
    /// Represents the "Entity" in the Entity Component System architecture.
    /// 
    /// It is a combination of an index and version. While this index can increase up
    /// to (2^32)-1, it is generally recommended to set a maximum entity count (todo).
    /// There are internal containers and tracking structures which grow linearly with
    /// the number of Entities, and thus your machine is likely to run out of memory
    /// long before you actually reach the theoretical limit, even if many of the
    /// entities are dead.
    /// 
    /// The version is used to minimize how quickly the index grows. Whenever an entity
    /// is destroyed, it's index is "released" and is available for a new entity. When
    /// the index is reused the version is incremented. While the index may be a
    /// sequential identifier, the version is a temporal one.
    /// 
    /// In this way to correctly identify an entity, you must compare both the index
    /// and the version.
    /// 
    /// The version is particularly useful, as the bulk of entities may actually be
    /// very fleeting in nature. For example particle systems, projectiles, etc. may
    /// create (and destroy) thousands of entities per second. Without a version, this
    /// would quickly balloon the aforementioned internal tracking structures. Instead
    /// the indices are reused and memory usage is stabilized.
    /// 
    /// 32-bit unsigned integers are used for both the index and version. This is done
    /// intentionally for several reasons. While it is unlikely one would be able to
    /// (or need to) instantiate over 4 billion entities, the other alternative to
    /// use a 16-bit uint would only allow for 65 thousand entities. Likely too few
    /// except for the simplest of applications.
    /// 
    /// A single 32-bit uint could be split into say a 20-bit index and 12-bit version,
    /// which may be sufficient for most use-cases. However this would introduce the 
    /// overhead of bit masking (and shifting if version is needed) when doing any
    /// indexing with the entity, which may occur millions of times per frame.
    /// 
    /// Finally, a combined 64-bit entity fits perfectly with the fact that modern
    /// CPUs are highly optimized to work with 64-bit scalars.
    /// </summary>
    struct Entity
    {
        /// <summary>
        /// The index of this Entity. Used to map it to its components. 
        /// </summary>
        uint32_t index;

        /// <summary>
        /// The current version/generation of this Entity. After an Entity is destroyed,
        /// it's index may be reused. When the index is reused the version increases.
        /// </summary>
        uint32_t version;
    };
}

#endif