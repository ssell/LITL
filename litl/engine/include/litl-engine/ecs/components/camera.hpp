#ifndef LITL_ENGINE_ECS_COMPONENTS_CAMERA_H__
#define LITL_ENGINE_ECS_COMPONENTS_CAMERA_H__

#include "litl-core/math/bounds.hpp"
#include "litl-core/math/types.hpp"
#include "litl-ecs/register.hpp"

namespace litl
{
    /// <summary>
    /// Transient data related to a specific camera.
    /// This is not stored with the Camera component itself but is instead owned by the active scene.
    /// </summary>
    struct CameraData
    {
        mat4 viewMatrix{};
        mat4 projMatrix{};
        mat4 viewProjMatrix{};
        bounds::Frustum frustum{};
        vec3 worldPosition{};
    };

    /// <summary>
    /// This component signifies that the attached entity is a camera and performs rendering to a target.
    /// </summary>
    struct Camera
    {
        /// <summary>
        /// If true, this is the main camera - the one drawing to the swapchain.
        /// </summary>
        bool mainCamera = false;

        /// <summary>
        /// If ture, the camera is participating in some form of rendering and 
        /// will be processed by the appropriate systems such as culling, etc.
        /// </summary>
        bool enabled = true;
    };
}


LITL_REGISTER_COMPONENT(litl::Camera);
LITL_REGISTER_COMPONENT(litl::Camera);


#endif