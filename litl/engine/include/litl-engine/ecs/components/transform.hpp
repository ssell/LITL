#ifndef LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__
#define LITL_ENGINE_ECS_COMPONENTS_TRANSFORM_H__

#include "litl-core/math.hpp"
#include "litl-ecs/register.hpp"
#include "litl-ecs/world.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-ecs/entity/parentEntity.hpp"

namespace litl
{
    /// <summary>
    /// The local transform of an entity.
    /// If the entity has no parent (null), then this is also the world transform.
    /// 
    /// Note that Transform uses getters and setters for its members. This is intentional
    /// as it has internal version tracking which is used for performance improvements.
    /// </summary>
    struct Transform
    {
        [[nodiscard]] quat getRotation() const noexcept
        {
            return rotation;
        }

        [[nodiscard]] vec3 getPosition() const noexcept
        {
            return position;
        }

        [[nodiscard]] float getUniformScale() const noexcept
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

        [[nodiscard]] ParentEntity const& getParent() const noexcept
        {
            return parent;
        }

        [[nodiscard]] ParentEntity& getParent() noexcept
        {
            return parent;
        }

        [[nodiscard]] mat4 getWorldMatrix() const noexcept
        {
            mat4 worldMatrix{};

            const float x2 = rotation.x() + rotation.x();
            const float y2 = rotation.y() + rotation.y();
            const float z2 = rotation.z() + rotation.z();

            const float xx = rotation.x() * x2, xy = rotation.x() * y2, xz = rotation.x() * z2;
            const float yy = rotation.y() * y2, yz = rotation.y() * z2, zz = rotation.z() * z2;
            const float wx = rotation.w() * x2, wy = rotation.w() * y2, wz = rotation.w() * z2;

            worldMatrix.setCol(0, { uniformScale * (1.0f - (yy + zz)), uniformScale * (xy + wz),          uniformScale * (xz - wy),          0.0f });
            worldMatrix.setCol(1, { uniformScale * (xy - wz),          uniformScale * (1.0f - (xx + zz)), uniformScale * (yz + wx),          0.0f });
            worldMatrix.setCol(2, { uniformScale * (xz + wy),          uniformScale * (yz - wx),          uniformScale * (1.0f - (xx + yy)), 0.0f });
            worldMatrix.setCol(3, { position.x(),                      position.y(),                      position.z(),                      1.0f });

            return worldMatrix;
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

}

LITL_REGISTER_COMPONENT(litl::Transform);

#endif