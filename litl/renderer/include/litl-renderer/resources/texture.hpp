#ifndef LITL_RENDERER_TEXTURE_H__
#define LITL_RENDERER_TEXTURE_H__

#include <cstdint>

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct TextureDescriptor
    {
        /// <summary>
        /// Width of the texture, in pixels.
        /// </summary>
        uint32_t width = 0u;

        /// <summary>
        /// Height of the texture, in pixels.
        /// </summary>
        uint32_t height = 0u;

        /// <summary>
        /// Format of the image.
        /// </summary>
        DataFormat format = DataFormat::Undefined;

        /// <summary>
        /// Optional name for the texture.
        /// If specified, it needs to be unique (or the original resource path).
        /// Used for hotreloads and debugging.
        /// </summary>
        std::string name;
    };

    struct TextureTag {};
    using TextureHandle = Handle<TextureTag>;
}

#endif