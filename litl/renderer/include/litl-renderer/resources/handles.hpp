#ifndef LITL_RENDERER_HANDLES_H__
#define LITL_RENDERER_HANDLES_H__

#include "litl-core/handles.hpp"

namespace litl
{
    struct CommandBufferHandleTag {};
    using CommandBufferHandle = Handle<CommandBufferHandleTag>;

    struct ComputePipelineHandleTag {};
    using ComputePipelineHandle = Handle<ComputePipelineHandleTag>;

    struct GraphicsPipelineHandleTag {};
    using GraphicsPipelineHandle = Handle<GraphicsPipelineHandleTag>;

    struct SamplerHandleTag {};
    using SamplerHandle = Handle<SamplerHandleTag>;

    struct ShaderModuleHandleTag {};
    using ShaderModuleHandle = Handle<ShaderModuleHandleTag>;

    struct TextureResourceHandleTag {};
    using TextureResourceHandle = Handle<TextureResourceHandleTag>;
}

LITL_STD_HASH_HANDLE(litl::CommandBufferHandleTag);
LITL_STD_HASH_HANDLE(litl::ComputePipelineHandleTag);
LITL_STD_HASH_HANDLE(litl::GraphicsPipelineHandleTag);
LITL_STD_HASH_HANDLE(litl::SamplerHandleTag);
LITL_STD_HASH_HANDLE(litl::ShaderModuleHandleTag);
LITL_STD_HASH_HANDLE(litl::TextureResourceHandleTag);

#endif