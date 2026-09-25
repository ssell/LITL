#include "litl-renderer-vulkan/resources/utility/destructionQueue.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"

namespace litl::vulkan
{
    void DestructionQueue::build(VkDevice vkDevice, VmaAllocator vmaAllocator, uint32_t frameDelay) noexcept
    {
        m_vkDevice = vkDevice;
        m_vmaAllocator = vmaAllocator;
        m_frameDelay = litl::max(frameDelay, 1u);
    }

    void DestructionQueue::process() noexcept
    {
        LITL_ASSERT_MSG((m_vkDevice != VK_NULL_HANDLE), "DestructionQueue::process invoked while m_vkDevice is NULL", );

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
                    vkDestroyPipeline(m_vkDevice, item.vkPipeline, nullptr);
                    break;

                case DestructionResourceType::ShaderModule:
                    vkDestroyShaderModule(m_vkDevice, item.vkShaderModule, nullptr);
                    break;

                case DestructionResourceType::Buffer:
                    vmaDestroyBuffer(m_vmaAllocator, item.destructionBuffer.vkBuffer, item.destructionBuffer.vmaAllocation);
                    break;

                case DestructionResourceType::SampledImage:
                    vkDestroyImageView(m_vkDevice, item.destructionImage.vkImageView, nullptr);
                    vkDestroyImage(m_vkDevice, item.destructionImage.vkImage, nullptr);
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

    void DestructionQueue::enqueue(VkBuffer vkBuffer, VmaAllocation vmaAllocation) noexcept
    {
        m_toDestroy.push_back(DestructionItem{
            .type = DestructionResourceType::Buffer,
            .frames = m_frameDelay,
            .destructionBuffer = DestructionBuffer {
                .vkBuffer = vkBuffer,
                .vmaAllocation = vmaAllocation
            }
        });
    }

    void DestructionQueue::enqueue(VkImage vkImage, VkImageView vkImageView) noexcept
    {
        m_toDestroy.push_back(DestructionItem{
            .type = DestructionResourceType::SampledImage,
            .frames = m_frameDelay,
            .destructionImage = DestructionImage {
                .vkImage = vkImage,
                .vkImageView = vkImageView
            }
        });
    }
}