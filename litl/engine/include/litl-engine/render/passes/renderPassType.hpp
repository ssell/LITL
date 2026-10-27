#ifndef LITL_ENGINE_RENDER_PASS_TYPE_H__
#define LITL_ENGINE_RENDER_PASS_TYPE_H__

#include <cstdint>

namespace litl
{
    enum class RenderPassType : uint8_t
    {
        ForwardOpaque = 0u,
        Count
    };
}

#endif