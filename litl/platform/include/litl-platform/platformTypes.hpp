#ifndef LITL_PLATFORM_TYPES_H__
#define LITL_PLATFORM_TYPES_H__

#include <array>
#include <string_view>

namespace litl
{
    enum class PlatformType : uint32_t
    {
        None = 0u,
        Glfw = 1u
    };

    inline constexpr std::array<std::string_view, 2> PlatformNames = {
        "None",
        "GLFW"
    };
}

#endif