#ifndef LITL_ENGINE_ECS_COMPONENTS_VELOCITY_H__
#define LITL_ENGINE_ECS_COMPONENTS_VELOCITY_H__

#include <glm/glm.hpp>
#include "litl-core/types.hpp"

namespace LITL::ECS
{
    struct Velocity
    {
        glm::vec3 direction;
        float speed;
    };
}

REGISTER_TYPE_NAME(LITL::ECS::Velocity)

#endif