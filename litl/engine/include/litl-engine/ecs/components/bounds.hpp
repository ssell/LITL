#ifndef LITL_ENGINE_ECS_COMPONENTS_BOUNDS_H__
#define LITL_ENGINE_ECS_COMPONENTS_BOUNDS_H__

#include "litl-core/math/bounds.hpp"
#include "litl-ecs/constants.hpp"


namespace litl
{
    struct Bounds
    {
        bounds::AABB bounds{};
    };
}

REGISTER_TYPE_NAME(litl::Bounds);

#endif