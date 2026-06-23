#ifndef LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__

#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/pipelineResource.hpp"

namespace litl::vulkan
{
    struct GraphicsPipelineResource
    {
        /// <summary>
        /// The Vulkan pipeline handle.
        /// </summary>
        VkPipeline vkPipeline = VK_NULL_HANDLE;

        /// <summary>
        /// The pipeline interface that this graphics pipeline was built against.
        /// </summary>
        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

        /// <summary>
        /// Map of resources bound in this pipeline.
        /// </summary>
        PipelineResourceMap resourceMap{};

        /// <summary>
        /// The descriptor that was used to create the pipeline.
        /// Must be retained so that the pipeline can be updated on a shader hot reload.
        /// </summary>
        GraphicsPipelineDescriptor descriptor;
    };
}

#endif