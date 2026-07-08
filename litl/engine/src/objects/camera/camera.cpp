#include "litl-engine/objects/camera/camera.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/cameraRef.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneCameras.hpp"

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

    void Camera::update(Authority<Scene> authority, mat4 worldMatrix) noexcept
    {
        m_worldMatrix = worldMatrix;
        m_viewMatrix = worldMatrix.inverse();
        m_viewProjMatrix = m_projMatrix * m_viewMatrix;
        m_worldPosition = worldMatrix.position();
        m_frustum = bounds::Frustum::fromViewProjection(
            m_viewProjMatrix, 
            bounds::FrustumExtractionOptions{
                .useInfiniteFar = m_isMain
            });
    }

    void Camera::setAsMainCamera(Authority<SceneCameras> authority, bool main) noexcept
    {
        m_isMain = main;
    }

    bool Camera::isMainCamera() const noexcept
    {
        return m_isMain;
    }

    mat4 const& Camera::getWorldMatrix() const noexcept
    {
        return m_worldMatrix;
    }

    mat4 const& Camera::getViewMatrix() const noexcept
    {
        return m_viewMatrix;
    }

    void Camera::setProjectionMatrix(mat4 const& projMatrix) noexcept
    {
        m_projMatrix = projMatrix;
    }

    mat4 const& Camera::getProjectionMatrix() const noexcept
    {
        return m_projMatrix;
    }

    mat4 const& Camera::getViewProjectionMatrix() const noexcept
    {
        return m_viewProjMatrix;
    }

    vec3 Camera::getWorldPosition() const noexcept
    {
        return m_worldPosition;
    }

    bounds::Frustum const& Camera::getFrustum() const noexcept
    {
        return m_frustum;
    }

    Entity Camera::getEntity() const noexcept
    {
        return m_entity;
    }

    CameraDescriptor const& Camera::getDescriptor() const noexcept
    {
        return m_descriptor;
    }
}