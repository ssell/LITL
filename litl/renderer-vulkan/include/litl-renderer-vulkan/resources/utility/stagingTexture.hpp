#ifndef LITL_RENDERER_VULKAN_STAGING_TEXTURE_H__
#define LITL_RENDERER_VULKAN_STAGING_TEXTURE_H__

#include <optional>
#include <span>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/resources/texture.hpp"
#include "litl-renderer-vulkan/resources/commandBuffer.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    /// <summary>
    /// 
    /// </summary>
    class StagingTexture final
    {
    public:

        StagingTexture();
        ~StagingTexture();

        StagingTexture(StagingTexture const&) = delete;
        StagingTexture& operator=(StagingTexture const&) = delete;

        void build(RendererContext& context) noexcept;

        [[nodiscard]] std::optional<uint64_t> copyIntoStaging(TextureDimensions dimensions, DataFormat format, uint32_t width, uint32_t height, uint32_t depth, std::span<std::byte const> source) noexcept;
        [[nodiscard]] bool copyIntoDestination(CommandBufferResource* commandBuffer, uint64_t stagingIndex, TextureResource* destination) noexcept;
        void flushTextures(CommandBufferResource* commandBuffer);
        void freeTextures() noexcept;

    private:

        TextureHandle createStagingTexture(TextureDimensions dimensions, DataFormat format, uint32_t width, uint32_t height, uint32_t depth) noexcept;
        void flushTexture(CommandBufferResource* commandBuffer, TextureResource* resource) noexcept;

        RendererContext* m_pContext;
        std::vector<TextureHandle> m_stagingTextures;
    };
}

#endif