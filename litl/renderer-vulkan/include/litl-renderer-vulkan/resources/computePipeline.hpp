#ifndef LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__
#define LITL_RENDERER_VULKAN_COMPUTE_PIPELINE_H__

#include "litl-renderer/resources/computePipeline.hpp"
#include "litl-renderer-vulkan/resources/pipeline.hpp"

namespace litl::vulkan
{
    struct ComputePipelineResource
    {
        /// <summary>
        /// The general pipeline data (shared with GrahpicsPipelineResource).
        /// </summary>
        PipelineResource pipeline{};

        /// <summary>
        /// The descriptor that was used to create the pipeline.
        /// Must be retained so that the pipeline can be updated on a shader hot reload.
        /// </summary>
        ComputePipelineDescriptor descriptor;
    };
}

#endif