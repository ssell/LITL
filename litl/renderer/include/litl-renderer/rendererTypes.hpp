#ifndef LITL_RENDERER_TYPES_H__
#define LITL_RENDERER_TYPES_H__

#include <string>

namespace litl
{
    enum class RendererBackendType : uint32_t
    {
        None = 0,
        Vulkan = 1,
        D3D12 = 2,
        Metal = 3
    };

    static char const* RendererBackendNames[] = {
        "None",
        "Vulkan",
        "D3D12",
        "Metal"
    };

    enum class PipelineBindType : uint32_t
    {
        Graphics = 0,
        Compute = 1
    };
}

#endif