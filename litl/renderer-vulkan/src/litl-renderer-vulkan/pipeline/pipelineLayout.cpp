#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace litl::vulkan
{
    litl::PipelineLayout* createPipelineLayout(VkDevice vkDevice, litl::PipelineLayoutDescriptor const& descriptor)
    {
        auto* pipelineLayoutHandle = new PipelineLayoutHandle {
            .vkDevice = vkDevice,
            .vkPipelineLayout = VK_NULL_HANDLE
        };

        return new litl::PipelineLayout(
            &VulkanPipelineLayoutOperations,
            LITL_PACK_HANDLE(litl::PipelineLayoutHandle, pipelineLayoutHandle),
            descriptor);
    }

    bool build(
        litl::PipelineLayoutDescriptor const& descriptor, 
        litl::PipelineLayoutHandle const& litlHandle)
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

    void destroy(litl::PipelineLayoutHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(PipelineLayoutHandle, litlHandle);

        if (handle->vkPipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(handle->vkDevice, handle->vkPipelineLayout, nullptr);
        }

        delete handle;
    }
}