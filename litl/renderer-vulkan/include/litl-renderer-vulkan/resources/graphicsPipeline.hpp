#ifndef LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__

#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/resources/pipeline.hpp"

namespace litl::vulkan
{
    struct GraphicsPipelineResource
    {
        /// <summary>
        /// The general pipeline data (shared with ComputePipelineResource).
        /// </summary>
        PipelineResource pipeline{};

        /// <summary>
        /// The descriptor that was used to create the pipeline.
        /// Must be retained so that the pipeline can be updated on a shader hot reload.
        /// </summary>
        GraphicsPipelineDescriptor descriptor;
    };
}

#endif