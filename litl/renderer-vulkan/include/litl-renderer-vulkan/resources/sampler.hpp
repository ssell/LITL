#ifndef LITL_RENDERER_VULKAN_SAMPLER_H__
#define LITL_RENDERER_VULKAN_SAMPLER_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/sampler.hpp"

namespace litl::vulkan
{
    struct SamplerResource
    {
        /// <summary>
        /// The Vulkan sampler handle.
        /// </summary>
        VkSampler vkSampler = VK_NULL_HANDLE;

        /// <summary>
        /// The descriptor that defined the sampler.
        /// </summary>
        SamplerDescriptor descriptor{};
    };
}

#endif