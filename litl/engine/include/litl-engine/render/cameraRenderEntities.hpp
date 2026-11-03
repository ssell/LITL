#ifndef LITL_ENGINE_RENDER_CAMERA_RENDER_ENTITIES_H__
#define LITL_ENGINE_RENDER_CAMERA_RENDER_ENTITIES_H__

#include "litl-core/constants.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-engine/render/renderableEntity.hpp"

namespace litl
{
    /// <summary>
    /// All visible renderable entities for a specific camera.
    /// </summary>
    struct CameraRenderEntities
    {
        Camera* camera = nullptr;

        /// <summary>
        /// Entities whose material blend mode is Opaque or Masked.
        /// These are typically sorted by material and mesh.
        /// </summary>
        std::vector<RenderableEntity> opaque;

        /// <summary>
        /// Entities whose material blend mode is Transparent.
        /// These are typically sorted by distance from the camera.
        /// </summary>
        std::vector<RenderableEntity> transparent;
    };
}

#endif