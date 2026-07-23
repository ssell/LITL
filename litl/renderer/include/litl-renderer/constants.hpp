#ifndef LITL_RENDERER_CONSTANTS_H__
#define LITL_RENDERER_CONSTANTS_H__

#include <cstdint>

namespace litl
{
    struct RendererConstants
    {
        /// <summary>
        /// A push-constant structure can not exceed this size (in bytes).
        /// </summary>
        static constexpr uint32_t MaxPushConstantSize = 128u;
    };
}

#endif