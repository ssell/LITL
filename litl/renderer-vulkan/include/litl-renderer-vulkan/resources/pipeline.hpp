#ifndef LITL_RENDERER_VULKAN_RESOURCES_PIPELINE_H__
#define LITL_RENDERER_VULKAN_RESOURCES_PIPELINE_H__

#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/pipelineResource.hpp"

namespace litl::vulkan
{
    struct PipelineResource
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
        /// One entry per set the pipeline declares.
        /// Note: these are owned by the pipeline cache, not by this resource.
        /// </summary>
        std::vector<VkDescriptorSetLayout> setLayouts;
    };
}

#endif