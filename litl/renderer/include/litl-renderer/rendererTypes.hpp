#ifndef LITL_RENDERER_TYPES_H__
#define LITL_RENDERER_TYPES_H__

#include <array>
#include <string_view>

namespace litl
{
    enum class RendererBackendType : uint32_t
    {
        None = 0,
        Vulkan = 1,
        D3D12 = 2,
        Metal = 3
    };

    inline constexpr std::array<std::string_view, 4> RendererBackendNames = {
        "None",
        "Vulkan",
        "D3D12",
        "Metal"
    };

    constexpr std::string_view getRendererName(RendererBackendType type) noexcept
    {
        const auto idx = static_cast<size_t>(type);

        if (idx < RendererBackendNames.size())
        {
            return RendererBackendNames[idx];
        }

        return "Unknown";
    }

    enum class PipelineBindType : uint32_t
    {
        Graphics = 0,
        Compute = 1
    };
}

#endif