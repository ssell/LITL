#ifndef LITL_ENGINE_ECS_CONSTANTS_H__
#define LITL_ENGINE_ECS_CONSTANTS_H__

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace LITL::ECS
{
    using ComponentTypeId = uint32_t;
    using StableComponentTypeId = uint64_t;
    using SystemTypeId = uint32_t;
    using ArchetypeId = uint32_t;

    struct Constants
    {
        static constexpr uint32_t null_entity_id = std::numeric_limits<uint32_t>::max();
        static constexpr ComponentTypeId null_component_id  = 0;
        static constexpr uint32_t max_component_size = 1024;
        static constexpr uint32_t max_components = 64;
        static constexpr uint32_t max_component_variants = 4096;
        static constexpr uint32_t chunk_size = 16384;
        static constexpr uint32_t max_entities_per_chunk = 512;
        static constexpr uint32_t entity_command_pool_size = 1024 * 16;
    };

    template<typename T>
    concept ValidComponentType = std::is_standard_layout_v<T> && sizeof(T) <= Constants::max_component_size;
}

#endif