#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    bool build(
        LITL::Renderer::PipelineLayoutData const& data,
        LITL::Renderer::PipelineLayoutDescriptor const& descriptor, 
        LITL::Renderer::PipelineLayoutHandle& handle)
    {
        auto* pipelineData = static_cast<PipelineLayoutData*>(data.handle);
        auto pipelineHandle = new PipelineLayoutHandle{ pipelineData->device, VK_NULL_HANDLE };

        // v todo pull from layout descriptor v
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        const VkResult result = vkCreatePipelineLayout(pipelineData->device, &pipelineLayoutInfo, nullptr, &pipelineHandle->pipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Pipeline Layout with result ", result);
            return false;
        }

        handle.handle = { pipelineHandle };
        return true;
    }

    void destroy(LITL::Renderer::PipelineLayoutHandle const& handle)
    {
        auto* pipelineLayoutHandle = static_cast<PipelineLayoutHandle*>(handle.handle);

        if (pipelineLayoutHandle->pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(pipelineLayoutHandle->device, pipelineLayoutHandle->pipelineLayout, nullptr);
        }

        delete pipelineLayoutHandle;
    }
}