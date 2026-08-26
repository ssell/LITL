#include "litl-renderer-vulkan/resources/utility/destructionQueue.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/assert.hpp"

namespace litl::vulkan
{
    void DestructionQueue::build(VkDevice vkDevice, VmaAllocator vmaAllocator) noexcept
    {
        m_vkDevice = vkDevice;
        m_vmaAllocator = vmaAllocator;
    }

    void DestructionQueue::process() noexcept
    {
        LITL_ASSERT_MSG((m_vkDevice != VK_NULL_HANDLE), "DestructionQueue::process invoked while m_vkDevice is NULL", );

        while (!m_toDestroy.empty())
        {
            auto item = m_toDestroy.front(); m_toDestroy.pop();

            switch (item.type)
            {
            case DestructionResourceType::Pipeline:
                vkDestroyPipeline(m_vkDevice, item.vkPipeline, nullptr);
                break;

            case DestructionResourceType::ShaderModule:
                vkDestroyShaderModule(m_vkDevice, item.vkShaderModule, nullptr);
                break;

            case DestructionResourceType::Buffer:
                vmaDestroyBuffer(m_vmaAllocator, item.destructionBuffer.vkBuffer, item.destructionBuffer.vmaAllocation);
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

    void DestructionQueue::enqueue(VkBuffer vkBuffer, VmaAllocation vmaAllocation) noexcept
    {
        m_toDestroy.push(DestructionItem{ 
            .type = DestructionResourceType::Buffer, 
            .destructionBuffer = DestructionBuffer {
                .vkBuffer = vkBuffer,
                .vmaAllocation = vmaAllocation
            }
        });
    }
}