#ifndef LITL_ENGINE_RENDER_PASS_TYPE_H__
#define LITL_ENGINE_RENDER_PASS_TYPE_H__

#include <cstdint>

namespace litl
{
    enum class RenderPassType : uint8_t
    {
        DepthOnly = 0u,
        Shadow = 1u,
        ForwardOpaque = 2u,
        ForwardTransparent = 3u,
        Blit = 4u,
        Count
    };
}

#endif