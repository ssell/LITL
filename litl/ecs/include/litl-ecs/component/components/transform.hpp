#ifndef LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__

#include <glm/glm.hpp>
#include "litl-core/types.hpp"

namespace LITL::ECS
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };
}

REGISTER_TYPE_NAME(LITL::ECS::Transform)

#endif