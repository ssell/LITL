#ifndef LITL_ENGINE_ECS_CONSTANTS_H__
#define LITL_ENGINE_ECS_CONSTANTS_H__

#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace LITL::ECS
{
    /// <summary>
    /// A runtime constant identifier for a component type.
    /// This is not guaranteed to be consistent between application runs.
    /// </summary>
    using ComponentTypeId = uint32_t;

    /// <summary>
    /// A globally constant identifier for a component type.
    /// This is guaranteed to be consistent between application runs.
    /// </summary>
    using StableComponentTypeId = uint64_t;

    /// <summary>
    /// A runtime constant identifier for a system type.
    /// This is not guaranteed to be consistent between application runs.
    /// </summary>
    using SystemTypeId = uint32_t;

    /// <summary>
    /// A runtime constant identifier for a system type.
    /// This is not guaranteed to be consistent between application runs.
    /// </summary>
    using ArchetypeId = uint32_t;

    struct Constants
    {
        /// <summary>
        /// Represents an uninitialized Entity.
        /// This does not signify that an Entity is destroyed. Only the World can determine that.
        /// </summary>
        static constexpr uint32_t null_entity_id = std::numeric_limits<uint32_t>::max();

        /// <summary>
        /// Represents an unset component unstable id.
        /// </summary>
        static constexpr ComponentTypeId null_component_id  = 0;

        /// <summary>
        /// Maximum size, in bytes, of a single component.
        /// This is an upper extreme, and components generally should be as small as possible
        /// to allow for more efficient entity packing within archetype chunks.
        /// </summary>
        static constexpr uint32_t max_component_size = 1024;

        /// <summary>
        /// Maximum number of components which can be attached to a single entity at a time.
        /// Which is the same as the maximum number of components which may comprise an archetype.
        /// </summary>
        static constexpr uint32_t max_components = 64;

        /// <summary>
        /// Maximum number of unique component definitions allowed in a single application run.
        /// </summary>
        static constexpr uint32_t max_component_types = 4096;

        /// <summary>
        /// The size, in bytes, of a single archetype chunk.
        /// </summary>
        static constexpr uint32_t chunk_size = 16384;

        /// <summary>
        /// The maximum number of entities which may be stored within a single archetype chunk.
        /// </summary>
        static constexpr uint32_t max_entities_per_chunk = 512;

        /// <summary>
        /// The size, in bytes, of a single deferred entity command pool.
        /// </summary>
        static constexpr uint32_t entity_command_pool_size = 1024 * 16;
    };

    template<typename T>
    concept ValidComponentType = std::is_standard_layout_v<T> && sizeof(T) <= Constants::max_component_size;
}

#endif