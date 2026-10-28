#include <atomic>
#include <format>

#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"
#include "litl-engine/objects/renderTexture.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/render/renderManager.hpp"

namespace litl
{
    namespace
    {
        std::atomic<uint32_t> s_rtCount{ 0u };

        void populateGenericTextureDescriptor(TextureDescriptor& texDesc, uint32_t width, uint32_t height) noexcept
        {
            texDesc.textureInfo.width = width;
            texDesc.textureInfo.height = height;
            texDesc.textureInfo.memory = BufferMemoryType::Auto;
            texDesc.textureInfo.memoryUsage = BufferMemoryUsage::GpuOnly;
            texDesc.textureInfo.sharing = SharingMode::Exclusive;
            texDesc.textureInfo.mipLevels = 1u;
            texDesc.textureInfo.arrayLayers = 1u;
            texDesc.textureInfo.faceCount = 1u;
            texDesc.textureInfo.sampleCount = MultisampleCount::Count1;
            texDesc.textureInfo.isCubeMap = false;
            texDesc.textureInfo.name = texDesc.objectInfo.name;
        }
    }

    [[nodiscard]] bool RenderTexture::create(Authority<ObjectPool> auth, RenderTextureDescriptor const& descriptor, ObjectPool& objectPool, RenderManager& renderManager) noexcept
    {
        m_descriptor = descriptor;
        m_pObjectPool = &objectPool;
        m_pRenderManager = &renderManager;

        if (!m_descriptor.colorComponent && !m_descriptor.depthComponent)
        {
            return false;
        }

        const auto* renderer = renderManager.getRenderer();

        if (renderer == nullptr)
        {
            return false;
        }

        const auto swapchainDimensions = renderer->getSwapchainDimensions();
        const auto rtId = s_rtCount.fetch_add(1u);

        switch (m_descriptor.sizePolicy)
        {
        case RenderTextureSizePolicy::Fixed:
            setDimensions(m_descriptor.fixedWidth, m_descriptor.fixedHeight);
            break;

        case RenderTextureSizePolicy::SwapchainRelative:
            setDimensions(ceil(m_descriptor.relativeWidth * static_cast<float>(swapchainDimensions.width)), ceil(m_descriptor.relativeHeight * static_cast<float>(swapchainDimensions.height)));
            logWarning("RenderTextureSizePolicy::SwapchainRelative is not fully implemented. Render texture will not resize with the swapchain.");
            break;

        default:
            logWarning("Unhandled RenderTexture size policy '", static_cast<uint32_t>(m_descriptor.sizePolicy), "'");
            break;
        }

        if (m_descriptor.colorComponent)
        {
            TextureDescriptor colorDescriptor{
                .objectInfo = ObjectDescriptor {
                    .name = std::format("{}_RT_Color_{}", m_descriptor.objectInfo.name, rtId),
                },
                .textureInfo = TextureResourceDescriptor {
                    .format = renderer->getSwapchainImageFormat(),
                    .usage = TextureUsageFlagBits::ColorAttachment | TextureUsageFlagBits::Sampled,
                    .residesInTextureTable = true
                }
            };

            populateGenericTextureDescriptor(colorDescriptor, m_currWidth, m_currHeight);
            m_colorTextureHandle = objectPool.createTexture(colorDescriptor);

            if (!m_colorTextureHandle.isValid())
            {
                logWarning("Failed to create RenderTexture color component for '", m_descriptor.objectInfo.name, "'");
                return false;
            }
        }

        if (m_descriptor.depthComponent)
        {
            TextureDescriptor depthDescriptor{
                .objectInfo = ObjectDescriptor {
                    .name = std::format("{}_RT_Depth_{}", m_descriptor.objectInfo.name, rtId)
                },
                .textureInfo = TextureResourceDescriptor {
                    .format = renderer->getSwapchainDepthFormat(),
                    .usage = TextureUsageFlagBits::DepthStencilAttachment | TextureUsageFlagBits::Sampled
                }
            };

            populateGenericTextureDescriptor(depthDescriptor, m_currWidth, m_currHeight);
            m_depthTextureHandle = objectPool.createTexture(depthDescriptor);

            if (!m_depthTextureHandle.isValid())
            {
                logWarning("Failed to create RenderTexture depth component for '", m_descriptor.objectInfo.name, "'");
                return false;
            }
        }

        return true;
    }

    void RenderTexture::setSelfHandle(Authority<ObjectPool> auth, RenderTextureHandle handle) noexcept
    {
        m_selfHandle = handle;
    }

    void RenderTexture::destroy(Authority<ObjectPool> auth) noexcept
    {
        if (m_pObjectPool == nullptr)
        {
            return;
        }

        if (m_colorTextureHandle.isValid())
        {
            m_pObjectPool->destroyTexture(m_colorTextureHandle);
            m_colorTextureHandle = {};
        }

        if (m_depthTextureHandle.isValid())
        {
            m_pObjectPool->destroyTexture(m_depthTextureHandle);
            m_depthTextureHandle = {};
        }

        m_selfHandle = {};
        m_pObjectPool = nullptr;
    }

    RenderTextureDescriptor const& RenderTexture::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    TextureHandle RenderTexture::getColorTextureHandle() const noexcept
    {
        return m_colorTextureHandle;
    }

    TextureHandle RenderTexture::getDepthTextureHandle() const noexcept
    {
        return m_depthTextureHandle;
    }

    RenderTextureSizePolicy RenderTexture::getSizePolicy() const noexcept
    {
        return m_descriptor.sizePolicy;
    }

    void RenderTexture::setDimensions(uint32_t w, uint32_t h) noexcept
    {
        m_currWidth = clamp(w, 1u, MaxDimensions);
        m_currHeight = clamp(h, 1u, MaxDimensions);
    }

    uint32_t RenderTexture::getWidth() const noexcept
    {
        return m_currWidth;
    }

    uint32_t RenderTexture::getHeight() const noexcept
    {
        return m_currHeight;
    }

    bool RenderTexture::resize(uint32_t width, uint32_t height) noexcept
    {
        if (m_descriptor.sizePolicy == RenderTextureSizePolicy::SwapchainRelative)
        {
            return false;
        }

        return resizeInternal(width, height);
    }

    bool RenderTexture::resize(Authority<RenderManager> auth, uint32_t width, uint32_t height) noexcept
    {
        return resizeInternal(width, height);
    }

    bool RenderTexture::resizeInternal(uint32_t width, uint32_t height) noexcept
    {
        if ((width == 0u) || (height == 0u))
        {
            return false;
        }

        if (m_descriptor.sizePolicy == RenderTextureSizePolicy::SwapchainRelative)
        {
            width = static_cast<uint32_t>(ceil(static_cast<float>(width) * m_descriptor.relativeWidth));
            height = static_cast<uint32_t>(ceil(static_cast<float>(height) * m_descriptor.relativeHeight));
        }

        width = clamp(width, 0u, MaxDimensions);
        height = clamp(height, 0u, MaxDimensions);

        if ((width == m_currWidth) && (height == m_currHeight))
        {
            return true;
        }

        Texture* colorTexture = m_pObjectPool->getTexture(m_colorTextureHandle);

        if (colorTexture != nullptr)
        {
            if (!colorTexture->resize(width, height))
            {
                logWarning("Failed to resize color texture for render texture '", m_descriptor.objectInfo.name, "'");
                return false;
            }
        }

        Texture* depthTexture = m_pObjectPool->getTexture(m_depthTextureHandle);

        if (depthTexture != nullptr)
        {
            if (!depthTexture->resize(width, height))
            {
                logWarning("Failed to resize depth texture for render texture '", m_descriptor.objectInfo.name, "'");
                return false;
            }
        }

        m_currWidth = width;
        m_currHeight = height;

        return true;
    }
}