#ifndef LITL_RENDERER_CONFIGURATION_H__
#define LITL_RENDERER_CONFIGURATION_H__

#include <cstdint>

#include "litl-core/constants.hpp"
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

        /// <summary>
        /// The size of the fixed portion of the staging ring buffer.
        /// This portion does not resize during application lifetime.
        /// </summary>
        uint32_t stagingBufferFixedSize = 32u * Constants::bytes_to_megabyte;

        void sanitize() noexcept;
    };
}

#endif