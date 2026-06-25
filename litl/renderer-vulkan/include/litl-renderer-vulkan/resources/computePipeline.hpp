#ifndef LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__
#define LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/computePipeline.hpp"
#include "litl-renderer/resources/pipelineResource.hpp"
#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"

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

        /// <summary>
        /// The pipeline interface that this compute pipeline was built against.
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

        /// <summary>
        /// Accumulated pending descriptor set changes for this pipeline.
        /// </summary>
        DescriptorSetChangeTracker descriptorSetChanges;
    };
}

#endif