#ifndef LITL_RENDERER_FIXED_PIPELINE_H__
#define LITL_RENDERER_FIXED_PIPELINE_H__

#include <cstdint>

namespace LITL::Renderer
{
    enum class CullMode : uint32_t
    {
        None      = 0b00000000,
        Front     = 0b00000001,
        Back      = 0b00000010,
        FrontBack = 0b00000011
    };

    enum class FillMode : int
    {
        Fill  = 0b00000000,
        Line  = 0b00000001,
        Point = 0b00000010
    };

    struct RasterState
    {
        CullMode cull;
        FillMode fill;
    };

    struct DepthStencilState
    {
        bool depthTest;
        bool depthWrite;
    };

    struct BlendState
    {
        bool enabled;
    };
}

#endif