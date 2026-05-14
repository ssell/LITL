#ifndef LITL_RENDERER_TYPES_H__
#define LITL_RENDERER_TYPES_H__

#include <string>

namespace litl
{
    enum class RendererBackendType : uint32_t
    {
        None = 0,
        Vulkan = 1
    };

    static char const* RendererBackendNames[] = {
        "None",
        "Vulkan"
    };

    enum class PipelineBindType : uint32_t
    {
        Graphics = 0,
        Compute = 1
    };
}

#endif