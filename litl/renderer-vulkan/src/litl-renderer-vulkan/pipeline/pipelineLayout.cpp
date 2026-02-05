#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    struct PipelineLayoutHandle
    {
        VkDevice device;
        VkPipelineLayout pipelineLayout;
    };

    std::unique_ptr<LITL::Renderer::PipelineLayout> createPipelineLayout(VkDevice vkDevice)
    {
        return std::make_unique<LITL::Renderer::PipelineLayout>(VulkanPipelineLayoutOperations, LITL::Renderer::PipelineLayoutHandle{
            new PipelineLayoutHandle{ vkDevice }
        });
    }

    bool build(
        LITL::Renderer::PipelineLayoutDescriptor const& descriptor, 
        LITL::Renderer::PipelineLayoutHandle const& litlHandle)
    {
        auto* pipelineLayoutHandle = LITL_UNPACK_HANDLE(PipelineLayoutHandle, litlHandle);

        // v todo pull from layout descriptor v
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        const VkResult result = vkCreatePipelineLayout(pipelineLayoutHandle->device, &pipelineLayoutInfo, nullptr, &pipelineLayoutHandle->pipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Pipeline Layout with result ", result);
            return false;
        }

        return true;
    }

    void destroy(LITL::Renderer::PipelineLayoutHandle const& litlHandle)
    {
        auto* pipelineLayoutHandle = LITL_UNPACK_HANDLE(PipelineLayoutHandle, litlHandle);

        if (pipelineLayoutHandle->pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(pipelineLayoutHandle->device, pipelineLayoutHandle->pipelineLayout, nullptr);
        }

        delete pipelineLayoutHandle;
    }
}