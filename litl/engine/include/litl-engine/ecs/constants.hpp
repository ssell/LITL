#ifndef LITL_ENGINE_ECS_CONSTANTS_H__
#define LITL_ENGINE_ECS_CONSTANTS_H__

#include <cstdint>

namespace LITL::Engine::ECS
{
    using ComponentTypeId = uint32_t;

    constexpr ComponentTypeId NULL_COMPONENT = 0;
    constexpr uint32_t MAX_COMPONENTS = 64;
    constexpr uint32_t CHUNK_SIZE_BYTES = 16384;
    constexpr uint32_t MAX_ENTITIES_PER_CHUNK = 512;
}

#endif