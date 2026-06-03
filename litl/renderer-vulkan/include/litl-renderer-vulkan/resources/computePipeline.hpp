#ifndef LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__
#define LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl::vulkan
{
    struct ComputePipelineResource
    {
        /// <summary>
        /// The descriptor that was used to create the pipeline.
        /// Must be retained so that the pipeline can be updated on a shader hot reload.
        /// </summary>
        ComputePipelineDescriptor descriptor;

        /// <summary>
        /// The Vulkan pipeline handle.
        /// </summary>
        VkPipeline vkPipeline = VK_NULL_HANDLE;
    };
}

#endif