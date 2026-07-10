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

    struct CameraDescriptor : ObjectDescriptor
    {
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

        friend class ObjectPool;

        void create(CameraDescriptor const& descriptor, World& world, CameraHandle handle) noexcept;
        void destroy() noexcept;

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