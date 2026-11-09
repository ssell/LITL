#ifndef LITL_RENDERER_TEXTURE_CHANNEL_H__
#define LITL_RENDERER_TEXTURE_CHANNEL_H__

#include <array>
#include <cstdint>
#include <string_view>

namespace litl
{
    enum class TextureChannel : uint8_t
    {
        All = 0u,
        R = 1u,
        G = 2u,
        B = 3u,
        A = 4u
    };

    constexpr std::array<std::string_view, 5u> TextureChannelName = { "RGBA", "R", "G", "B", "A" };
}

#endif