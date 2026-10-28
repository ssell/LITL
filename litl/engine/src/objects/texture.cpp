#include <format>

#include "litl-core/enumBitFlags.hpp"
#include "litl-core/math/textureUtils.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/objects/texture.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] bool requiresLocalPixelBuffer(TextureDescriptor const& descriptor) noexcept
        {
            return has_any(descriptor.textureInfo.usage, TextureUsageFlagBits::TransferDest);
        }
    }
    bool Texture::create(Authority<ObjectPool> auth, TextureDescriptor const& texDescriptor, RenderManager& renderManager) noexcept
    {
        m_descriptor = texDescriptor;
        m_pRenderManager = &renderManager;

        if (m_descriptor.textureInfo.name.empty())
        {
            m_descriptor.textureInfo.name = m_descriptor.objectInfo.name;
        }

        m_resourceHandle = renderManager.getRenderer()->createTexture(texDescriptor.textureInfo);

        if (!m_resourceHandle.isValid())
        {
            return false;
        }

        if (m_descriptor.textureInfo.residesInTextureTable)
        {
            m_textureTableIndex = renderManager.getRenderer()->getTextureTableIndex(m_resourceHandle);
        }

        if (requiresLocalPixelBuffer(m_descriptor))
        {
            resizePixelBuffer();
        }
        else
        {
            m_isDirty = false;
        }

        return true;
    }

    bool Texture::create(Authority<ObjectPool> auth, ObjectDescriptor const& objDescriptor, RenderManager& renderManager) noexcept
    {
        m_descriptor.objectInfo = objDescriptor;
        m_descriptor.textureInfo.name = m_descriptor.objectInfo.name;
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

        m_selfHandle = {};
        m_pRenderManager = nullptr;
        m_pixelBytes.clear();
        m_pixelBytes.shrink_to_fit();
    }

    void Texture::resizePixelBuffer() noexcept
    {
        uint64_t totalBytes = m_descriptor.textureInfo.arrayLayers;

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

    uint32_t Texture::getWidth() const noexcept
    {
        return m_descriptor.textureInfo.width;
    }

    uint32_t Texture::getHeight() const noexcept
    {
        return m_descriptor.textureInfo.height;
    }

    TextureDescriptor const& Texture::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    TextureResourceHandle Texture::getTextureResourceHandle() const noexcept
    {
        return m_resourceHandle;
    }

    uint32_t Texture::getTextureTableIndex() const noexcept
    {
        return m_textureTableIndex;
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

            if (m_resourceHandle.isValid())
            {
                if (m_descriptor.textureInfo.residesInTextureTable)
                {
                    m_textureTableIndex = m_pRenderManager->getRenderer()->getTextureTableIndex(m_resourceHandle);
                }
            }
            else
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
        buildTightlyPackedUploadRegions(m_descriptor.textureInfo, regions);

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

    bool Texture::resize(uint32_t width, uint32_t height) noexcept
    {
        if ((m_pRenderManager == nullptr) || (width == 0u) || (height == 0u))
        {
            return false;
        }

        if ((width == getWidth()) && (height == getHeight()))
        {
            return true;
        }

        const Renderer* renderer = m_pRenderManager->getRenderer();

        if (renderer == nullptr)
        {
            return false;
        }

        m_generation++;

        TextureResourceDescriptor resizedDescriptor = m_descriptor.textureInfo;
        resizedDescriptor.width = width;
        resizedDescriptor.height = height;
        resizedDescriptor.name = std::format("{}_{}", resizedDescriptor.name, m_generation);
        
        if (resizedDescriptor.residesInTextureTable)
        {
            resizedDescriptor.textureTableIndexOverride = m_textureTableIndex;
        }

        const TextureResourceHandle newHandle = renderer->createTexture(resizedDescriptor);

        if (!newHandle.isValid())
        {
            logWarning("Failed to resize texture '", m_descriptor.objectInfo.name, "'");
            return false;
        }

        // Only update internal state after successful creation of new underyling texture resource
        const TextureResourceHandle oldHandle = m_resourceHandle;
        m_descriptor.textureInfo.width = width;
        m_descriptor.textureInfo.height = height;
        m_resourceHandle = newHandle;

        if (requiresLocalPixelBuffer(m_descriptor))
        {
            resizePixelBuffer();
        }

        if (oldHandle.isValid())
        {
            renderer->destroyTexture(oldHandle);
        }

        return true;
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