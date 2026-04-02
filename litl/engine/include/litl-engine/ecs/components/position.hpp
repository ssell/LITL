#ifndef LITL_ENGINE_ECS_COMPONENTS_POSITION_H__
#define LITL_ENGINE_ECS_COMPONENTS_POSITION_H__

#include "litl-core/math/vec3.hpp"
#include "litl-core/types.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// CRTP inheritance of a three-component vector.
    /// </summary>
    struct Position : Math::Vec3Wrapper<Position> {
        using Vec3Wrapper::Vec3Wrapper;
    };
}

REGISTER_TYPE_NAME(LITL::Engine::Position)

#endif