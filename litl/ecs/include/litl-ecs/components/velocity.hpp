#ifndef LITL_ENGINE_ECS_COMPONENTS_VELOCITY_H__
#define LITL_ENGINE_ECS_COMPONENTS_VELOCITY_H__

#include <glm/glm.hpp>

namespace LITL::ECS
{
    struct Velocity
    {
        glm::vec3 direction;
        float speed;
    };
}

#endif