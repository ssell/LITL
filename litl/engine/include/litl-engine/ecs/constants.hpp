#ifndef LITL_ENGINE_ECS_CONSTANTS_H__
#define LITL_ENGINE_ECS_CONSTANTS_H__

#include <cstdint>

namespace LITL::Engine::ECS
{
    using ComponentTypeId = uint32_t;
    constexpr uint8_t NULL_COMPONENT = 0;
    constexpr uint8_t MAX_COMPONENTS = 64;
}

#endif