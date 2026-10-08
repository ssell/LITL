#ifndef LITL_RENDERER_CONSTANTS_H__
#define LITL_RENDERER_CONSTANTS_H__

#include <cstdint>

namespace litl
{
    struct RendererConstants
    {
        /// <summary>
        /// Typically should be 1-2.
        /// </summary>
        static constexpr uint32_t MaxFramesInFlight = 4u;

        /// <summary>
        /// A push-constant structure can not exceed this size (in bytes).
        /// </summary>
        static constexpr uint32_t MaxPushConstantSize = 128u;
    };
}

#endif