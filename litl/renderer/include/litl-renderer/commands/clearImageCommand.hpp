#ifndef LITL_RENDERER_COMMANDS_CLEAR_IMAGE_H__
#define LITL_RENDERER_COMMANDS_CLEAR_IMAGE_H__

#include "litl-core/math/types/color.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl
{
    struct ClearImageCommand
    {
        /// <summary>
        /// The color value to clear to.
        /// </summary>
        color clearColor;

        /// <summary>
        /// The texture/image to clear.
        /// If left to the default value, then the current swapchain image will be cleared.
        /// </summary>
        TextureResourceHandle image;

        /// <summary>
        /// 
        /// </summary>
        ImageLayoutType destLayout;

        /// <summary>
        /// The base layer index to clear.
        /// </summary>
        uint32_t layer = 0u;

        /// <summary>
        /// The number of layers to clear.
        /// </summary>
        uint32_t layerCount = 1u;
    };
}

#endif