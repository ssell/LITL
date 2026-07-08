#ifndef LITL_ENGINE_OBJECTS_CAMERA_H__
#define LITL_ENGINE_OBJECTS_CAMERA_H__

#include "litl-core/authority.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/camera/cameraDescriptor.hpp"

namespace litl
{
    class ObjectPool;
    class Scene;
    class SceneCameras;
    class World;

    class Camera
    {
    public:

        /// <summary>
        /// Retrieves the descriptor that was used to create this camera.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] CameraDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Retrieves the entity that represents this camera in the world.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] Entity getEntity() const noexcept;

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

    private:

        friend class ObjectPool;

        void create(CameraDescriptor const& descriptor, World& world, CameraHandle handle) noexcept;
        void destroy() noexcept;

        /// <summary>
        /// Is this camera the current main camera?
        /// </summary>
        bool m_isMain = false;

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