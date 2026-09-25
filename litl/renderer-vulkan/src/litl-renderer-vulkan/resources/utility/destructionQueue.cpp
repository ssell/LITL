#include "litl-renderer-vulkan/resources/utility/destructionQueue.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"

namespace litl::vulkan
{
    void DestructionQueue::build(RendererContext& rendererContext) noexcept
    {
        m_pContext = &rendererContext;
        m_frameDelay = m_pContext->renderInfo.frame.framesInFlight;
    }

    void DestructionQueue::process() noexcept
    {
        LITL_ASSERT_MSG((m_pContext != nullptr), "DestructionQueue::process invoked while RendererContext is NULL", );

        m_toDestroyIndices.reserve(m_toDestroy.size());
        m_toDestroyIndices.clear();

        for (size_t i = 0ull; i < m_toDestroy.size(); ++i)
        {
            auto& item = m_toDestroy[i];

            if (item.frames != 0u)
            {
                item.frames--;
            }
            else
            {
                m_toDestroyIndices.push_back(i);

                switch (item.type)
                {
                case DestructionResourceType::Pipeline:
                    vkDestroyPipeline(m_pContext->device.vkDevice, item.vkPipeline, nullptr);
                    break;

                case DestructionResourceType::ShaderModule:
                    vkDestroyShaderModule(m_pContext->device.vkDevice, item.vkShaderModule, nullptr);
                    break;

                case DestructionResourceType::Buffer:
                    m_pContext->resources.destroyBuffer(item.bufferHandle);
                    break;

                case DestructionResourceType::SampledImage:
                    m_pContext->resources.destroyTexture(item.textureHandle);
                    break;

                default:
                    LITL_LOG_WARNING_CAPTURE("DestructionQueue process skipping unhandled type of ", static_cast<uint32_t>(item.type));
                    break;
                }
            }
        }

        eraseVectorIndices(m_toDestroy, m_toDestroyIndices);
    }

    void DestructionQueue::enqueue(VkPipeline vkPipeline) noexcept
    {
        m_toDestroy.push_back(DestructionItem{
            .type = DestructionResourceType::Pipeline, 
            .frames = m_frameDelay,
            .vkPipeline = vkPipeline 
        });
    }

    void DestructionQueue::enqueue(VkShaderModule vkShaderModule) noexcept
    {
        m_toDestroy.push_back(DestructionItem{ 
            .type = DestructionResourceType::ShaderModule,
            .frames = m_frameDelay,
            .vkShaderModule = vkShaderModule 
        });
    }

    void DestructionQueue::enqueue(BufferHandle bufferHandle) noexcept
    {
        m_toDestroy.push_back(DestructionItem{
            .type = DestructionResourceType::Buffer,
            .frames = m_frameDelay,
            .bufferHandle = bufferHandle
        });
    }

    void DestructionQueue::enqueue(TextureResourceHandle textureHandle) noexcept
    {
        m_toDestroy.push_back(DestructionItem{
            .type = DestructionResourceType::SampledImage,
            .frames = m_frameDelay,
            .textureHandle = textureHandle
        });
    }
}