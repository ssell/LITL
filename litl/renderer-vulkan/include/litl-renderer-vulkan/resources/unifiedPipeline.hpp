#ifndef LITL_RENDERER_VULKAN_UNIFIED_PIPELINE_H__
#define LITL_RENDERER_VULKAN_UNIFIED_PIPELINE_H__

namespace litl::vulkan
{
    struct GraphicsPipelineResource;
    struct ComputePipelineResource;

    struct UnifiedPipelineHandle final
    {
        GraphicsPipelineResource* graphicsPipeline = nullptr;
        ComputePipelineResource* computePipeline = nullptr;
    };
}

#endif