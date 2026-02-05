#ifndef LITL_RENDERER_HANDLES_H__
#define LITL_RENDERER_HANDLES_H__

using LITLHandle = void*;

namespace LITL::Renderer
{
    struct PipelineHandle
    {
        LITLHandle handle = nullptr;
    };

#define LITL_UNPACK_HANDLE(HandleType, HandleObj) static_cast<HandleType*>(HandleObj.handle)
}

#endif