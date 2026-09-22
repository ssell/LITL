#ifndef LITL_CORE_TRANSFER_FUNCTION_H__
#define LITL_CORE_TRANSFER_FUNCTION_H__

#include <cstdint>

namespace litl
{
    enum class TransferFunction : uint8_t
    {
        /// <summary>
        /// Pixels are linear on the range (0-255) (0.0-1.0).
        /// Use for non-color textures such as normal maps, masks, roughness, etc.
        /// </summary>
        Linear = 0,

        /// <summary>
        /// Pixels are stored in sRGB gamma space which interpolates values more
        /// closely to how human eyes perceive them. For example, we see dark
        /// shades in more detail than bright ones.
        /// 
        /// Use for color textures such as albedo.
        /// 
        /// Most standard image formats such as JPEG, PNG, etc. store in sRGB.
        /// </summary>
        SRGB = 1
    };
}

#endif