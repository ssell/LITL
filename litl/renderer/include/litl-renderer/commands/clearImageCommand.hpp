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
        TextureHandle image;

        /// <summary>
        /// 
        /// </summary>
        ImageLayoutType destLayout;
    };
}

#endif