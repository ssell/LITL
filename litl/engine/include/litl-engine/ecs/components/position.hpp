#ifndef LITL_ENGINE_ECS_COMPONENTS_POSITION_H__
#define LITL_ENGINE_ECS_COMPONENTS_POSITION_H__

#include "litl-core/math/types.hpp"

namespace LITL::Engine
{
    struct Position 
    {
        vec3 value;
    };
}

REGISTER_TYPE_NAME(LITL::Engine::Position)

#endif