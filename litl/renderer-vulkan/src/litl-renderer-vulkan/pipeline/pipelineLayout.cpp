#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    PipelineLayout::PipelineLayout(VkDevice vkDevice, LITL::Renderer::PipelineLayoutDescriptor const& descriptor)
        : m_vkDevice(vkDevice), m_vkPipelineLayout(VK_NULL_HANDLE)
    {
        m_descriptor = descriptor;
    }

    PipelineLayout::~PipelineLayout()
    {
        cleanup();
    }

    void PipelineLayout::cleanup()
    {
        if (m_vkPipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_vkDevice, m_vkPipelineLayout, nullptr);
            m_vkPipelineLayout = VK_NULL_HANDLE;
        }
    }

    bool PipelineLayout::build() noexcept
    {
        if (m_vkPipelineLayout != VK_NULL_HANDLE)
        {
            return false;
        }

        // v todo pull from layout descriptor v
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        const VkResult result = vkCreatePipelineLayout(m_vkDevice, &pipelineLayoutInfo, nullptr, &m_vkPipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Pipeline Layout with result ", result);
            return false;
        }
        return true;
    }

    bool PipelineLayout::rebuild(LITL::Renderer::PipelineLayoutDescriptor const& descriptor) noexcept
    {
        cleanup();
        m_descriptor = descriptor;

        return build();
    }

    VkPipelineLayout PipelineLayout::getVkPipelineLayout() const noexcept
    {
        return m_vkPipelineLayout;
    }
}