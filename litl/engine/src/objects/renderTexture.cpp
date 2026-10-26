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

        void populateGenericTextureDescriptor(RenderTextureResourceDescriptor& rtTexDesc, TextureDescriptor& texDesc, SwapChainDimensions const& swapchainDimensions) noexcept
        {
            switch (rtTexDesc.sizePolicy)
            {
            case RenderTextureSizePolicy::Fixed:
                rtTexDesc.fixedWidth = clamp(rtTexDesc.fixedWidth, 1u, RenderTexture::MaxDimensions);
                rtTexDesc.fixedHeight = clamp(rtTexDesc.fixedHeight, 1u, RenderTexture::MaxDimensions);
                texDesc.textureInfo.width = rtTexDesc.fixedWidth;
                texDesc.textureInfo.height = rtTexDesc.fixedHeight;
                break;

            case RenderTextureSizePolicy::SwapchainRelative:
                texDesc.textureInfo.width = static_cast<uint32_t>(ceil(rtTexDesc.relativeWidth * static_cast<float>(swapchainDimensions.width)));
                texDesc.textureInfo.height = static_cast<uint32_t>(ceil(rtTexDesc.relativeHeight * static_cast<float>(swapchainDimensions.height)));
                logWarning("RenderTextureSizePolicy::SwapchainRelative is not fully implemented. Render texture will not resize with the swapchain.");
                break;

            default:
                logWarning("Unhandled RenderTexture size policy '", static_cast<uint32_t>(rtTexDesc.sizePolicy), "'");
                break;
            }

            texDesc.textureInfo.width = clamp(texDesc.textureInfo.width, 1u, RenderTexture::MaxDimensions);
            texDesc.textureInfo.height = clamp(texDesc.textureInfo.height, 1u, RenderTexture::MaxDimensions);
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
    [[nodiscard]] bool RenderTexture::create(Authority<ObjectPool> auth, RenderTextureDescriptor const& descriptor, ObjectPool& objectPool, RenderManager const& renderManager) noexcept
    {
        m_descriptor = descriptor;
        m_pObjectPool = &objectPool;

        if (!m_descriptor.color.has_value() && !m_descriptor.depth.has_value())
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

        if (m_descriptor.color.has_value())
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

            populateGenericTextureDescriptor(m_descriptor.color.value(), colorDescriptor, swapchainDimensions);

            m_colorTextureHandle = objectPool.createTexture(colorDescriptor);

            if (!m_colorTextureHandle.isValid())
            {
                logWarning("Failed to create RenderTexture color component for '", m_descriptor.objectInfo.name, "'");
                return false;
            }
        }

        if (m_descriptor.depth.has_value())
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

            populateGenericTextureDescriptor(m_descriptor.depth.value(), depthDescriptor, swapchainDimensions);

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

    TextureHandle RenderTexture::getColorTextureHandle() const noexcept
    {
        return m_colorTextureHandle;
    }

    TextureHandle RenderTexture::getDepthTextureHandle() const noexcept
    {
        return m_depthTextureHandle;
    }
}