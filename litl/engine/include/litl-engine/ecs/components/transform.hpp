#ifndef LITL_ENGINE_ECS_COMPONENT_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENT_TRANSFORM_H__

#include <glm/glm.hpp>

namespace LITL::Engine::ECS::Component
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };
}

#endif