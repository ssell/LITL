#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    LITL::Renderer::PipelineLayout* createPipelineLayout(VkDevice vkDevice, LITL::Renderer::PipelineLayoutDescriptor const& descriptor)
    {
        auto* pipelineLayoutHandle = new PipelineLayoutHandle {
            .vkDevice = vkDevice,
            .vkPipelineLayout = VK_NULL_HANDLE
        };

        return new LITL::Renderer::PipelineLayout(
            &VulkanPipelineLayoutOperations,
            LITL_PACK_HANDLE(LITL::Renderer::PipelineLayoutHandle, pipelineLayoutHandle),
            descriptor);
    }

    bool build(
        LITL::Renderer::PipelineLayoutDescriptor const& descriptor, 
        LITL::Renderer::PipelineLayoutHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(PipelineLayoutHandle, litlHandle);

        if (handle->vkPipelineLayout != VK_NULL_HANDLE)
        {
            // already exists
            return true;
        }

        // v todo pull from layout descriptor v
        const VkPipelineLayoutCreateInfo pipelineLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 0,
            .pushConstantRangeCount = 0
        };

        const VkResult result = vkCreatePipelineLayout(handle->vkDevice, &pipelineLayoutInfo, nullptr, &handle->vkPipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Pipeline Layout with result ", result);
            return false;
        }

        return true;
    }

    void destroy(LITL::Renderer::PipelineLayoutHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(PipelineLayoutHandle, litlHandle);

        if (handle->vkPipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(handle->vkDevice, handle->vkPipelineLayout, nullptr);
        }

        delete handle;
    }
}