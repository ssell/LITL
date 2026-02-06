#ifndef LITL_RENDERER_RENDERER_TYPES_H__
#define LITL_RENDERER_RENDERER_TYPES_H__

#include <string>

namespace LITL::Renderer
{
    enum RendererBackendType
    {
        None = 0,
        Vulkan = 1
    };

    static char const* RendererBackendNames[] = {
        "None",
        "Vulkan"
    };

    enum PipelineBindType
    {
        Graphics = 0,
        Compute = 1
    };
}

#endif