#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingTexture.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    StagingTexture::StagingTexture()
        : m_pContext(nullptr)
    {
        m_stagingTextures.reserve(32ull);
    }

    StagingTexture::~StagingTexture()
    {

    }

    void StagingTexture::build(RendererContext& context) noexcept
    {
        m_pContext = &context;
    }

    std::optional<uint64_t> StagingTexture::copyIntoStaging(TextureDimensions dimensions, DataFormat format, uint32_t width, uint32_t height, uint32_t depth, std::span<std::byte const> source) noexcept
    {
        // ... todo ...

        return {};
    }

    bool StagingTexture::copyIntoDestination(CommandBufferResource* commandBuffer, uint64_t stagingIndex, TextureResource* destination) noexcept
    {
        // ... todo ...

        return true;
    }

    void StagingTexture::flushTextures(CommandBufferResource* commandBuffer) noexcept
    {
        LITL_ASSERT_MSG((commandBuffer != nullptr), "Invalid command buffer provided to StagingTexture::flushTextures", );

        for (auto& textureHandle : m_stagingTextures)
        {
            flushTexture(commandBuffer, m_pContext->resources.getTexture(textureHandle));
        }
    }

    void StagingTexture::flushTexture(CommandBufferResource* commandBuffer, TextureResource* texture) noexcept
    {
        // ... todo ...
    }

    void StagingTexture::freeTextures() noexcept
    {
        for (auto& textureHandle : m_stagingTextures)
        {
            m_pContext->resources.destroyTexture(textureHandle);
        }
    }

    TextureHandle StagingTexture::createStagingTexture(TextureDimensions dimensions, DataFormat format, uint32_t width, uint32_t height, uint32_t depth) noexcept
    {
        TextureDescriptor descriptor{
            .dimensions = dimensions,
            .width = (width == 0u ? 1u : width),            // Caller may set to "0" intending to mean "unused", but 0 is never a valid value.
            .height = (height == 0u ? 1u : height),
            .depth = (depth == 0u ? 1u : depth),
            .format = format,
            .memory = BufferMemoryType::Auto,
            .memoryUsage = BufferMemoryUsage::Staging
        };

        return m_pContext->resources.createTexture(descriptor);
    }
}