#ifndef LITL_ENGINE_OBJECTS_CAMERA_H__
#define LITL_ENGINE_OBJECTS_CAMERA_H__

#include "litl-core/math/types.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-engine/objects/camera/cameraDescriptor.hpp"

namespace litl
{
    class ObjectPool;

    class Camera
    {
    public:

        /// <summary>
        /// Retrieves the descriptor that was used to create this camera.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] CameraDescriptor const& getDescriptor() const noexcept;

    private:

        friend class ObjectPool;

        void create(CameraDescriptor const& descriptor) noexcept;
        void destroy() noexcept;

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
    };
}

#endif