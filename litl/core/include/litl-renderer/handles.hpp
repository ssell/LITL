#ifndef LITL_RENDERER_HANDLES_H__
#define LITL_RENDERER_HANDLES_H__

using LITLHandle = void*;

namespace LITL::Renderer
{
#define DEFINE_LITL_HANDLE(Name) struct Name { LITLHandle handle; }
#define LITL_PACK_HANDLE(HandleType, HandleObj) HandleType { HandleObj }
#define LITL_UNPACK_HANDLE(HandleType, HandleObj) static_cast<HandleType*>(HandleObj.handle)
#define LITL_UNPACK_HANDLE_PTR(HandleType, HandlePtr) static_cast<HandleType*>(HandlePtr->handle)

    DEFINE_LITL_HANDLE(RendererHandle);
    DEFINE_LITL_HANDLE(PipelineLayoutHandle);
    DEFINE_LITL_HANDLE(GraphicsPipelineHandle);
    DEFINE_LITL_HANDLE(ComputePipelineHandle);
    DEFINE_LITL_HANDLE(CommandBufferHandle);
}

#endif