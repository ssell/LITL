#include "litl-engine/objects/camera/camera.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/cameraRef.hpp"

namespace litl
{
    void Camera::create(CameraDescriptor const& descriptor, World& world, CameraHandle handle) noexcept
    {
        m_descriptor = descriptor;
        m_entity = world.createImmediate();

        world.addComponentsImmediate<Transform, CameraRef>(
            m_entity, 
            Transform{}, 
            CameraRef{ .handle = handle }
        );
    }

    void Camera::destroy() noexcept
    {
        // ... todo? ...
    }

    CameraDescriptor const& Camera::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    Entity Camera::getEntity() const noexcept
    {
        return m_entity;
    }
}