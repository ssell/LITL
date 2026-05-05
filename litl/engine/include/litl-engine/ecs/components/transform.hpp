#ifndef LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__

#include "litl-core/math.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-ecs/entity/parentEntity.hpp"

namespace litl
{
    /// <summary>
    /// The local transform of an entity.
    /// If the entity has no parent (null), then this is also the world transform.
    /// </summary>
    struct Transform
    {
        quat getRotation() const noexcept
        {
            return rotation;
        }

        vec3 getPosition() const noexcept
        {
            return position;
        }

        float getUniformScale() const noexcept
        {
            return uniformScale;
        }

        void setRotation(quat rot)
        {
            rotation = rot;
            version = World::getVersion();
        }

        void setPosition(vec3 pos)
        {
            position = pos;
            version = World::getVersion();
        }

        void setUniformScale(float scale)
        {
            uniformScale = scale;
            version = World::getVersion();
        }

        [[nodiscard]] uint32_t getVersion() const noexcept
        {
            return version;
        }

        ParentEntity const& getParent() const noexcept
        {
            return parent;
        }

        ParentEntity& getParent() noexcept
        {
            return parent;
        }

    private:

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
        /// The current version of the transform.
        /// 
        /// This is incremented when the transform values are updated
        /// and is used to keep the WorldBounds in sync.
        /// </summary>
        uint32_t version{ 0u };

        /// <summary>
        /// The hierarchal parent of this entity in the scene.
        /// If null, then the entity has no parent and the world itself is the parent.
        /// </summary>
        ParentEntity parent{};
    };

    static_assert(ValidComponentType<litl::Transform>);
}

REGISTER_TYPE_NAME(litl::Transform);

#endif