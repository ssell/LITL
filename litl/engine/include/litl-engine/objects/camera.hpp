#ifndef LITL_ENGINE_OBJECTS_CAMERA_H__
#define LITL_ENGINE_OBJECTS_CAMERA_H__

#include "litl-core/authority.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class ObjectPool;
    class Scene;
    class SceneView;
    class SceneCameras;
    class World;

    /// <summary>
    /// Predefined selection of camera processing order values.
    /// Lower values are processed first.
    /// </summary>
    enum class CameraProcessOrder : uint32_t
    {
        /// <summary>
        /// Use for cameras that are to be rendered/processed before the main camera.
        /// </summary>
        Preprocess = 50u,

        /// <summary>
        /// The process position enforced for main camera.
        /// </summary>
        MainCamera = 100u,

        /// <summary>
        /// Use for cameras that are to be rendered/processed after the main camera.
        /// </summary>
        Postprocess = 150u
    };

    enum class CameraProjection : uint32_t
    {
        Perspective = 0u,
        Orthographic = 1u
    };

    /// <summary>
    /// Series of fields related to configuring a perspective projection.
    /// </summary>
    struct PerspectiveDescriptor
    {
        /// <summary>
        /// The field-of-view on the y-axis in radians.
        /// </summary>
        float fieldOfViewY = degreesToRadians(60.0f);

        /// <summary>
        /// The aspect ratio of the perspective projection.
        /// This should be render target (width / height).
        /// 
        /// If the camera is the main camera, then this is set automatically to match the swapchain render target.
        /// </summary>
        float aspectRatio = 1.0f;
    };

    /// <summary>
    /// Series of fields related to configuring an orthographic projection.
    /// </summary>
    struct OrthographicDescriptor
    {
        /// <summary>
        /// The left side of the frustum.
        /// </summary>
        float left = -10.0f;

        /// <summary>
        /// The right side of the frustum.
        /// </summary>
        float right = 10.0f;

        /// <summary>
        /// The bottom of the frustum.
        /// </summary>
        float bottom = -10.0f;

        /// <summary>
        /// The top of the frustum.
        /// </summary>
        float top = 10.0f;
    };

    struct CameraDescriptor : ObjectDescriptor
    {
        /// <summary>
        /// The projection type that the camera is using.
        /// </summary>
        CameraProjection projection = CameraProjection::Perspective;

        /// <summary>
        /// Relevant settings if the projection is set to Perspective.
        /// </summary>
        PerspectiveDescriptor perspective{};

        /// <summary>
        /// Relevant settings if the projection is set to Orthographic.
        /// </summary>
        OrthographicDescriptor orthographic{};

        /// <summary>
        /// The near clip-plane of the frustum in world units.
        /// </summary>
        float zNear = 0.0f;

        /// <summary>
        /// The far clip-plane of the frustum in world units.
        /// </summary>
        float zFar = 1000.0f;

        /// <summary>
        /// The priority level for the camera.
        /// Cameras with a lower level are processed first.
        /// </summary>
        uint32_t processOrder = static_cast<uint32_t>(CameraProcessOrder::Postprocess);
    };

    class Camera
    {
    public:

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="descriptor"></param>
        /// <param name="world"></param>
        /// <param name="handle"></param>
        void create(Authority<ObjectPool> auth, CameraDescriptor const& descriptor, World& world, SceneView& sceneView, CameraHandle handle) noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="auth"></param>
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Updates the cameras view matrix, world position, etc. with the provided world matrix.
        /// </summary>
        /// <param name="worldMatrix"></param>
        void update(Authority<Scene> authority, mat4 worldMatrix) noexcept;

        /// <summary>
        /// Sets this camera if it is or is not the main camera.
        /// </summary>
        /// <param name="authority"></param>
        /// <param name="main"></param>
        void setAsMainCamera(Authority<SceneCameras> authority, bool main) noexcept;

        /// <summary>
        /// Is this camera the current main camera?
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] bool isMainCamera() const noexcept;

        /// <summary>
        /// Sets the y-axis field-of-view (in radians) for the camera.
        /// Only relevant for perspective projections. 
        /// </summary>
        /// <param name="radians"></param>
        void setFieldOfView(float radians) noexcept;

        /// <summary>
        /// Retrieves the y-axis field-of-view (in radians) for the camera.
        /// Only relevant for perspective projections.
        /// </summary>
        [[nodiscard]] float getFieldOfView() const noexcept;

        /// <summary>
        /// Sets the aspect ratio (width/height) of the camera.
        /// Only relevant for perspective projections.
        /// </summary>
        /// <param name="aspectRatio"></param>
        void setAspectRatio(float aspectRatio) noexcept;

        /// <summary>
        /// Gets the aspect ratio (width/height) of the camera.
        /// Only relevant for perspective projections.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] float getAspectRatio() const noexcept;

        /// <summary>
        /// Sets the orthographic projection bounds of the camera.
        /// Only relevant for orthographic projections.
        /// </summary>
        /// <param name="left"></param>
        /// <param name="right"></param>
        /// <param name="bottom"></param>
        /// <param name="top"></param>
        void setOrthographicBounds(float left, float right, float bottom, float top) noexcept;

        /// <summary>
        /// Retrieves the orthographic bounds (left, right, bottom, top) of the camera.
        /// Only relevant for orthographic projections.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] vec4 getOrthographicBounds() const noexcept;

        /// <summary>
        /// Retrieves the world matrix of the camera.
        /// This may be up to one frame old as it is updated only once during PreRender.
        /// </summary>
        /// <returns></returns>
        mat4 const& getWorldMatrix() const noexcept;
        
        /// <summary>
        /// Retrieves the view matrix of the camera.
        /// This may be up to one frame old as it is updated only once during PreRender.
        /// </summary>
        /// <returns></returns>
        mat4 const& getViewMatrix() const noexcept;

        /// <summary>
        /// Sets the projection matrix of the camera.
        /// This may take up to a frame to propagate to the view-projection matrix and frustum
        /// as the camera is updated only once per frame during PreRender.
        /// </summary>
        /// <param name="projMatrix"></param>
        void setProjectionMatrix(mat4 const& projMatrix) noexcept;

        /// <summary>
        /// Retrieves the projection matrix of the camera.
        /// </summary>
        /// <returns></returns>
        mat4 const& getProjectionMatrix() const noexcept;

        /// <summary>
        /// Retrieves the view-projection matrix of the camera.
        /// This may be up to one frame old as it is updated only once during PreRender.
        /// </summary>
        /// <returns></returns>
        mat4 const& getViewProjectionMatrix() const noexcept;

        /// <summary>
        /// Retrieves the world position of the camera.
        /// This may be up to one frame old as it is updated only once during PreRender.
        /// </summary>
        /// <returns></returns>
        vec3 getWorldPosition() const noexcept;

        /// <summary>
        /// Retrieves the viewing frustum of the camera.
        /// This may be up to one frame old as it is updated only once during PreRender.
        /// </summary>
        /// <returns></returns>
        bounds::Frustum const& getFrustum() const noexcept;

        /// <summary>
        /// Retrieves the entity that represents this camera in the world.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] Entity getEntity() const noexcept;

        /// <summary>
        /// Returns the process order position for the camera.
        /// Lower values are processed first.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t getProcessPosition() const noexcept;

        /// <summary>
        /// Retrieves the descriptor that was used to create this camera.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] CameraDescriptor const& getDescriptor() const noexcept;

    private:

        /// <summary>
        /// Rebuilds the projection matrix.
        /// </summary>
        void rebuildProjectionMatrix() noexcept;

        /// <summary>
        /// Is this camera the current main camera?
        /// </summary>
        bool m_isMain = false;

        /// <summary>
        /// The order that the camera is processed. Lower values are processed first.
        /// This is set at time of creation (via the descriptor) unless the camera is marked as the main camera.
        /// When a camera is set as the main camera it uses the hardcoded CameraProcessOrder::MainCamera value.
        /// If a camera is unset from being the main camera then it reverts back to its original value.
        /// </summary>
        uint32_t m_processPosition = 0u;

        /// <summary>
        /// The world matrix of the camera. Updated once per frame during PreRender.
        /// </summary>
        mat4 m_worldMatrix{};

        /// <summary>
        /// The view matrix (inverse world matrix) of the camera. Updated once per frame during PreRender.
        /// </summary>
        mat4 m_viewMatrix{};

        /// <summary>
        /// The projection matrix of the camera.
        /// </summary>
        mat4 m_projMatrix{};

        /// <summary>
        /// The view-projection matrix (projection * view). Updated once per frame during PreRender.
        /// </summary>
        mat4 m_viewProjMatrix{};

        /// <summary>
        /// The culling frustum. Updated once per frame during PreRender.
        /// </summary>
        bounds::Frustum m_frustum{};

        /// <summary>
        /// The current world-space position of the camera. Updated once per frame during PreRender.
        /// </summary>
        vec3 m_worldPosition{};

        /// <summary>
        /// The descriptor that created this camera.
        /// </summary>
        CameraDescriptor m_descriptor{};

        /// <summary>
        /// The entity associated with this camera.
        /// </summary>
        Entity m_entity{};
    };
}

#endif