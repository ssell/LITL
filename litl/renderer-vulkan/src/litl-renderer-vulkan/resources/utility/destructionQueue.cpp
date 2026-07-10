#include "litl-renderer-vulkan/resources/utility/destructionQueue.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/assert.hpp"

namespace litl::vulkan
{
    void DestructionQueue::build(VkDevice vkDevice) noexcept
    {
        m_vkDevice = vkDevice;
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

            default:
                LITL_LOG_WARNING_CAPTURE("DestructionQueue process skipping unhandled type of ", static_cast<uint32_t>(item.type));
                break;
            }
        }
    }

    void DestructionQueue::enqueue(VkPipeline vkPipeline) noexcept
    {
        m_toDestroy.push(DestructionItem{ .type = DestructionResourceType::Pipeline, .vkPipeline = vkPipeline });
    }

    void DestructionQueue::enqueue(VkShaderModule vkShaderModule) noexcept
    {
        m_toDestroy.push(DestructionItem{ .type = DestructionResourceType::ShaderModule, .vkShaderModule = vkShaderModule });
    }
}