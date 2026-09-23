#include "litl-core/logging/logging.hpp"
#include "litl-engine/objects/texture.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-core/math/textureUtils.hpp"

namespace litl
{
    bool Texture::create(Authority<ObjectPool> auth, TextureDescriptor const& texDescriptor, RenderManager& renderManager) noexcept
    {
        m_descriptor = texDescriptor;
        m_pRenderManager = &renderManager;
        m_resourceHandle = renderManager.getRenderer()->createTexture(texDescriptor.textureInfo);

        if (!m_resourceHandle.isValid())
        {
            return false;
        }

        resizePixelBuffer();

        return true;
    }

    bool Texture::create(Authority<ObjectPool> auth, ObjectDescriptor const& objDescriptor, RenderManager& renderManager) noexcept
    {
        m_descriptor.objectInfo = objDescriptor;
        m_pRenderManager = &renderManager;

        return true;
    }

    void Texture::setSelfHandle(Authority<ObjectPool> auth, TextureHandle handle) noexcept
    {
        m_selfHandle = handle;
    }

    void Texture::destroy(Authority<ObjectPool> auth) noexcept
    {
        if (m_pRenderManager != nullptr)
        {
            if (m_resourceHandle.isValid())
            {
                m_pRenderManager->getRenderer()->destroyTexture(m_resourceHandle);
                m_resourceHandle = {};
            }
        }

        m_pRenderManager = nullptr;
        m_pixelBytes.clear();
        m_pixelBytes.shrink_to_fit();
    }

    void Texture::resizePixelBuffer() noexcept
    {
        uint64_t totalBytes = m_descriptor.textureInfo.arrayLayers * m_descriptor.textureInfo.faceCount;

        if (m_descriptor.textureInfo.mipLevels == 1u)
        {
            totalBytes *= imageLevelBytes(m_descriptor.textureInfo.format, m_descriptor.textureInfo.width, m_descriptor.textureInfo.height, m_descriptor.textureInfo.depth);
        }
        else
        {
            totalBytes *= imageChainBytes(m_descriptor.textureInfo.format, m_descriptor.textureInfo.width, m_descriptor.textureInfo.height, m_descriptor.textureInfo.depth);
        }

        m_pixelBytes.resize(totalBytes, std::byte{ 0 });
    }

    TextureDescriptor const& Texture::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    void Texture::updateDescriptor(Authority<TextureAsset> auth, TextureResourceDescriptor const& resourceDescriptor, bool persistOnCpu) noexcept
    {
        m_descriptor.textureInfo = resourceDescriptor;
        m_descriptor.persistOnCpu = persistOnCpu;

        resizePixelBuffer();
    }

    bool Texture::setPixelBytes(std::span<std::byte const> pixelBytes) noexcept
    {
        if (!isValidTextureWrite())
        {
            return false;
        }

        if (pixelBytes.size() != m_pixelBytes.size())
        {
            logWarning("setPixelBytes called with source data that does not match internal pixel buffer size on Texture '", m_descriptor.objectInfo.name, "'");
            return false;
        }

        m_pixelBytes.clear();
        m_pixelBytes.assign(pixelBytes.begin(), pixelBytes.end());
        m_isDirty = true;

        return true;
    }

    bool Texture::apply(std::optional<CommandBufferHandle> commandBuffer) noexcept
    {
        if (!m_isDirty)
        {
            return true;
        }

        if (!m_resourceHandle.isValid())
        {
            m_resourceHandle = m_pRenderManager->getRenderer()->createTexture(m_descriptor.textureInfo);

            if (!m_resourceHandle.isValid())
            {
                logError("apply called for Texture '", m_descriptor.objectInfo.name, "' but failed to create underlying texture resource.");
                return false;
            }
        }

        if (commandBuffer.has_value() && commandBuffer.value().isValid())
        {
            flushData(commandBuffer.value());
        }
        else
        {
            m_pRenderManager->trackDirtyTexture({}, m_selfHandle);
        }

        return true;
    }

    void Texture::flushData(Authority<RenderManager> auth, CommandBufferHandle commandBuffer) noexcept
    {
        flushData(commandBuffer);
    }

    void Texture::flushData(CommandBufferHandle commandBuffer) noexcept
    {
        if (!commandBuffer.isValid())
        {
            logError("flushData invoked with invalid command buffer handle on Texture '", m_descriptor.objectInfo.name, "'");
            return;
        }

        m_isDirty = false;

        std::vector<TextureUploadRegion> regions;
        regions.reserve(m_descriptor.textureInfo.mipLevels);
        uint64_t offset = 0ull;

        for (uint32_t i = 0; i < m_descriptor.textureInfo.mipLevels; ++i)
        {
            const uint32_t regionWidth = mipExtent(m_descriptor.textureInfo.width, i);
            const uint32_t regionHeight = mipExtent(m_descriptor.textureInfo.height, i);
            const uint32_t regionDepth = mipExtent(m_descriptor.textureInfo.depth, i);

            regions.push_back(TextureUploadRegion{
                .sourceOffset = offset,
                .mipLevel = i,
                .arrayLayer = 0u,               // update when adding support for array layers
                .width = regionWidth,
                .height = regionHeight,
                .depth = regionDepth
            });

            offset += imageLevelBytes(m_descriptor.textureInfo.format, regionWidth, regionHeight, regionDepth);
        }

        const auto result = m_pRenderManager->getRenderer()->cmdTextureUpload(commandBuffer, m_pixelBytes, regions, m_resourceHandle);

        if (result != RendererResult::Success)
        {
            logError("flushData failed with result ", static_cast<uint32_t>(result), " for Texture '", m_descriptor.objectInfo.name, "'");
        }

        if (!m_descriptor.persistOnCpu)
        {
            m_pixelBytes.clear();
            m_pixelBytes.shrink_to_fit();
        }
    }

    bool Texture::isValidTextureWrite() const noexcept
    {
        if (m_pixelBytes.empty())
        {
            logWarning("Attempting to write pixel data on Texture '", m_descriptor.objectInfo.name, "' with uninitialized and /or unpersisted CPU - side memory.");
            return false;
        }

        return true;
    }
}