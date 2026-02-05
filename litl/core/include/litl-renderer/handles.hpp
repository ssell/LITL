#ifndef LITL_RENDERER_HANDLES_H__
#define LITL_RENDERER_HANDLES_H__

using LITLHandle = void*;

namespace LITL::Renderer
{
    struct PipelineHandle
    {
        LITLHandle handle = nullptr;
    };
}

#endif