#ifndef LITL_RENDERER_COMPUTE_PIPELINE_H__
#define LITL_RENDERER_COMPUTE_PIPELINE_H__

#include "litl-core/handles.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct ComputePipelineDescriptor
    {
        PipelineShaderDescriptor compute;

        // ... todo ...

        PipelineShaderDescriptor* getShaderDescriptor(ShaderStage stage) noexcept
        {
            switch (stage)
            {
            case ShaderStage::Compute:
                return &compute;

            default:
                return nullptr;
            }
        }
    };

    struct ComputePipelineTag {};
    using ComputePipelineHandle = Handle<ComputePipelineTag>;
}

#endif