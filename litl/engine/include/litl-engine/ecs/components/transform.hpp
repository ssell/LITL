#ifndef LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__

#include <glm/glm.hpp>

namespace LITL::Engine::ECS
{
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };

    
}

#endif