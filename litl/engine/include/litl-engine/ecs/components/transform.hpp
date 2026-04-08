#ifndef LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__

#include "litl-core/math.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-ecs/entity/parentEntity.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// The local transform of an entity.
    /// If the entity has no parent (null), then this is also the world transform.
    /// </summary>
    struct Transform
    {
        /// <summary>
        /// The local rotation of the entity.
        /// </summary>
        quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

        /// <summary>
        /// The local position of the entity.
        /// </summary>
        vec3 position{ 0.0f, 0.0f, 0.0f };

        /// <summary>
        /// The local uniform scale of the entity.
        /// If a non-uniform scale is needed, then use the NonUniformScale component.
        /// </summary>
        float uniformScale{ 1.0f };

        /// <summary>
        /// The hierarchal parent of this entity in the scene.
        /// If null, then the entity has no parent and the world itself is the parent.
        /// </summary>
        ECS::ParentEntity parent{};
    };
}

REGISTER_TYPE_NAME(LITL::Engine::Transform);

#endif