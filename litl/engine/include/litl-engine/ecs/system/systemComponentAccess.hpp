#ifndef LITL_ENGINE_ECS_SYSTEM_COMPONENT_ACCESS_H__
#define LITL_ENGINE_ECS_SYSTEM_COMPONENT_ACCESS_H__

#include "litl-engine/ecs/constants.hpp"

namespace LITL::Engine::ECS
{
    enum class SystemComponentAccessType : uint8_t
    {
        None      = 0b0000'0000,
        Read      = 0b0000'0001,
        Write     = 0b0000'0010,
        ReadWrite = 0b0000'0011
    };

    struct SystemComponentAccess
    {
        ComponentTypeId component;
        SystemComponentAccessType access;
    };
}

#endif