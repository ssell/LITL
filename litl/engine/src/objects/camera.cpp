#include "litl-engine/objects/camera.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/ecs/components/cameraRef.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/scene/sceneCameras.hpp"

namespace litl
{
    void Camera::create(Authority<ObjectPool> auth, CameraDescriptor const& descriptor, World& world, SceneView& sceneView, CameraHandle handle) noexcept
    {
        m_descriptor = descriptor;
        m_processPosition = descriptor.processOrder;
        m_entity = world.createImmediate();

        rebuildProjectionMatrix();

        world.addComponentsImmediate<Transform, CameraRef>(
            m_entity, 
            Transform{},
            CameraRef{ .handle = handle }
        );

        sceneView.track(m_entity, world.getComponent<Transform>(m_entity).value());
    }

    void Camera::destroy(Authority<ObjectPool> auth) noexcept
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

    void Camera::rebuildProjectionMatrix() noexcept
    {
        switch (m_descriptor.projection)
        {
        case CameraProjection::Perspective:
            m_projMatrix = mat4::perspective(
                m_descriptor.perspective.fieldOfViewY,
                m_descriptor.perspective.aspectRatio,
                m_descriptor.zNear,
                m_descriptor.zFar);
            break;

        case CameraProjection::Orthographic:
            m_projMatrix = mat4::orthographic(
                m_descriptor.orthographic.left,
                m_descriptor.orthographic.right,
                m_descriptor.orthographic.bottom,
                m_descriptor.orthographic.top,
                m_descriptor.zNear,
                m_descriptor.zFar);
            break;
        }
    }

    void Camera::setAsMainCamera(Authority<SceneCameras> authority, bool main) noexcept
    {
        m_isMain = main;

        if (main)
        {
            m_processPosition = static_cast<uint32_t>(CameraProcessOrder::MainCamera);
        }
        else
        {
            m_processPosition = m_descriptor.processOrder;
        }
    }

    bool Camera::isMainCamera() const noexcept
    {
        return m_isMain;
    }

    void Camera::setFieldOfView(float radians) noexcept
    {
        LITL_ASSERT_MSG((radians > 0.0f), "Camera field-of-view must be positive and greater than 0.", );
        m_descriptor.perspective.fieldOfViewY = radians;
        rebuildProjectionMatrix();
    }

    float Camera::getFieldOfView() const noexcept
    {
        return m_descriptor.perspective.fieldOfViewY;
    }

    void Camera::setAspectRatio(float aspectRatio) noexcept
    {
        LITL_ASSERT_MSG((aspectRatio > 0.0f), "Camera aspect ratio must be positive and greater than 0.", );
        m_descriptor.perspective.aspectRatio = aspectRatio;
        rebuildProjectionMatrix();
    }

    float Camera::getAspectRatio() const noexcept
    {
        return m_descriptor.perspective.aspectRatio;
    }

    void Camera::setOrthographicBounds(float left, float right, float bottom, float top) noexcept
    {
        m_descriptor.orthographic.left = left;
        m_descriptor.orthographic.right = right;
        m_descriptor.orthographic.bottom = bottom;
        m_descriptor.orthographic.top = top;
    }

    vec4 Camera::getOrthographicBounds() const noexcept
    {
        return { m_descriptor.orthographic.left, m_descriptor.orthographic.right, m_descriptor.orthographic.bottom, m_descriptor.orthographic.top };
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

    uint32_t Camera::getProcessPosition() const noexcept
    {
        return m_processPosition;
    }

    CameraDescriptor const& Camera::getDescriptor() const noexcept
    {
        return m_descriptor;
    }
}