#ifndef LITL_RENDERER_CONFIGURATION_H__
#define LITL_RENDERER_CONFIGURATION_H__

#include <cstdint>

#include "litl-core/constants.hpp"
#include "litl-renderer/rendererTypes.hpp"

namespace litl
{
    struct RendererDescriptorSetConfiguration
    {
        /// <summary>
        /// The number of descriptor sets stored in an individual descriptor pool.
        /// </summary>
        uint32_t setsPerPool = 64u;

        /// <summary>
        /// The number of Uniform Buffer Objects (UBO) per pool.
        /// </summary>
        uint32_t uboCount = 64u;

        /// <summary>
        /// The number of Shader Storage Buffer Objects (SSBO) per pool.
        /// </summary>
        uint32_t ssboCount = 256u;

        /// <summary>
        /// The number of sampled textures per pool.
        /// </summary>
        uint32_t textureCount = 256u;

        /// <summary>
        /// The number of samplers per pool.
        /// </summary>
        uint32_t samplerCount = 16u;
    };

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
        /// The size of the fixed portion of the staging buffer memory arena.
        /// This portion does not resize during application lifetime.
        /// </summary>
        uint32_t stagingBufferFixedSize = 32u * Constants::bytes_to_megabyte;

        /// <summary>
        /// The size of the fixed portion of the staging texture memory arena.
        /// This portion does not resize during application lifetime.
        /// </summary>
        uint32_t stagingTextureFixedSize = 32u * Constants::bytes_to_megabyte;

        /// <summary>
        /// 
        /// </summary>
        RendererDescriptorSetConfiguration descriptorSet{};

        void sanitize() noexcept;
    };
}

#endif