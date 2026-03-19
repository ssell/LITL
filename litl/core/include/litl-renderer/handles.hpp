#ifndef LITL_RENDERER_HANDLES_H__
#define LITL_RENDERER_HANDLES_H__

#include "litl-core/handles.hpp"

namespace LITL::Renderer
{
    DEFINE_LITL_HANDLE(RendererHandle);
    DEFINE_LITL_HANDLE(PipelineLayoutHandle);
    DEFINE_LITL_HANDLE(GraphicsPipelineHandle);
    DEFINE_LITL_HANDLE(ComputePipelineHandle);
    DEFINE_LITL_HANDLE(CommandBufferHandle);
    DEFINE_LITL_HANDLE(ShaderModuleHandle);
}

#endif