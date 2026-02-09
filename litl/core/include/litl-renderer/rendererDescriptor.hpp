#ifndef LITL_RENDERER_RENDERER_DESCRIPTOR_H__
#define LITL_RENDERER_RENDERER_DESCRIPTOR_H__

#include <cstdint>
#include "litl-renderer/rendererTypes.hpp"

namespace LITL::Renderer
{
    struct RendererDescriptor
    {
        /// <summary>
        /// The library used by the renderer backend.
        /// </summary>
        LITL::Renderer::RendererBackendType rendererType = RendererBackendType::Vulkan;

        /// <summary>
        /// How many frames to have queued at a time. Suggested 2-3.
        /// </summary>
        uint32_t framesInFlight = 2;
    };
}

#endif