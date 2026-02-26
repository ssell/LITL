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

    template<typename T>
    concept ValidComponentType = std::is_standard_layout_v<T>;

    constexpr ComponentTypeId NULL_COMPONENT = 0;
    constexpr uint32_t MAX_COMPONENTS = 64;
    constexpr uint32_t MAX_COMPONENT_VARIANTS = 4096;
    constexpr uint32_t CHUNK_SIZE_BYTES = 16384;
    constexpr uint32_t MAX_ENTITIES_PER_CHUNK = 512;
}

#endif