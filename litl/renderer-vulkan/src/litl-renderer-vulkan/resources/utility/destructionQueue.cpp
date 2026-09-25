#include "litl-renderer-vulkan/resources/utility/destructionQueue.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl::vulkan
{
    void DestructionQueue::build(RendererContext& rendererContext) noexcept
    {
        m_pContext = &rendererContext;
    }

    void DestructionQueue::process() noexcept
    {
        LITL_ASSERT_MSG((m_pContext != nullptr), "DestructionQueue::process invoked while RendererContext is NULL", );

        while (!m_toDestroy.empty())
        {
            auto item = m_toDestroy.front(); m_toDestroy.pop();

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

    void DestructionQueue::enqueue(VkPipeline vkPipeline) noexcept
    {
        m_toDestroy.push(DestructionItem{
            .type = DestructionResourceType::Pipeline, 
            .vkPipeline = vkPipeline 
        });
    }

    void DestructionQueue::enqueue(VkShaderModule vkShaderModule) noexcept
    {
        m_toDestroy.push(DestructionItem{
            .type = DestructionResourceType::ShaderModule,
            .vkShaderModule = vkShaderModule 
        });
    }

    void DestructionQueue::enqueue(BufferHandle bufferHandle) noexcept
    {
        m_toDestroy.push(DestructionItem{
            .type = DestructionResourceType::Buffer,
            .bufferHandle = bufferHandle
        });
    }

    void DestructionQueue::enqueue(TextureResourceHandle textureHandle) noexcept
    {
        m_toDestroy.push(DestructionItem{
            .type = DestructionResourceType::SampledImage,
            .textureHandle = textureHandle
        });
    }
}