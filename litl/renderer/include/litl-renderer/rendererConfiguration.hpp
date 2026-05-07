#ifndef LITL_RENDERER_RENDERER_DESCRIPTOR_H__
#define LITL_RENDERER_RENDERER_DESCRIPTOR_H__

#include <cstdint>
#include "litl-renderer/rendererTypes.hpp"

namespace litl
{
    struct RendererConfiguration
    {
        /// <summary>
        /// The library used by the renderer backend.
        /// </summary>
        RendererBackendType rendererType = RendererBackendType::Vulkan;

        /// <summary>
        /// How many frames to have queued at a time. Suggested 2-3.
        /// </summary>
        uint32_t framesInFlight = 2;

        void sanitize() noexcept;
    };
}

#endif