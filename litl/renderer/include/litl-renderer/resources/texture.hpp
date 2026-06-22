#ifndef LITL_RENDERER_TEXTURE_H__
#define LITL_RENDERER_TEXTURE_H__

#include <cstdint>
#include <string>

#include "litl-core/handles.hpp"
#include "litl-renderer/enums.hpp"

namespace litl
{
    struct TextureDescriptor
    {
        /// <summary>
        /// The dimensionality of the texture.
        /// </summary>
        TextureDimensions dimensions = TextureDimensions::Texture2D;

        /// <summary>
        /// Width of the texture, in pixels.
        /// </summary>
        uint32_t width = 1u;

        /// <summary>
        /// Height of the texture, in pixels.
        /// </summary>
        uint32_t height = 1u;

        /// <summary>
        /// Depth of the texture in pixels.
        /// </summary>
        uint32_t depth = 1u;

        /// <summary>
        /// Format of the image.
        /// </summary>
        DataFormat format = DataFormat::Undefined;

        /// <summary>
        /// Where the memory for the buffer resides.
        /// </summary>
        BufferMemoryType memory = BufferMemoryType::Auto;

        /// <summary>
        /// How the memory will be accessed.
        /// </summary>
        BufferMemoryUsage memoryUsage = BufferMemoryUsage::GpuOnly;

        /// <summary>
        /// How the buffer can be shared between queue families.
        /// </summary>
        SharingMode sharing = SharingMode::Exclusive;

        /// <summary>
        /// The number of levels of detail available for minified sampling of the image.
        /// Set to 1 for a standard texture without any mipmaps.
        /// </summary>
        uint32_t mipLevels = 1u;

        /// <summary>
        /// If a texture array, the number of array layers.
        /// Set to 1 for a standard, single-layered texture.
        /// </summary>
        uint32_t arrayLayers = 1u;

        /// <summary>
        /// The number of samples per pixel. Primarily used for anti-aliasing.
        /// Set to a count of 1 for no multisampling.
        /// </summary>
        MultisampleCount sampleCount = MultisampleCount::Count1;

        /// <summary>
        /// Optional name for the texture.
        /// If specified, it needs to be unique (or the original resource path).
        /// Used for hotreloads and debugging.
        /// </summary>
        std::string name;
    };

    struct TextureTag {};
    using TextureHandle = Handle<TextureTag>;

    struct MappedTexture
    {
        /// <summary>
        /// If the texture is mapped, this is the CPU address of the start of its memory block.
        /// </summary>
        void* mappedPtr = nullptr;
    };
}

#endif