#ifndef LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/graphicsPipeline.hpp"

namespace litl::vulkan
{
    struct GraphicsPipelineHandle
    {
        VkDevice vkDevice;
        VkFormat vkSwapChainImageFormat;
        VkPipeline vkPipeline;
    };

    litl::GraphicsPipeline* createGraphicsPipeline(VkDevice vkDevice, VkFormat vkSwapChainImageFormat, litl::GraphicsPipelineDescriptor const& descriptor);

    bool build(litl::GraphicsPipelineDescriptor const& descriptor, litl::GraphicsPipelineHandle const& litlHandle);
    void destroy(litl::GraphicsPipelineHandle const& litlHandle);

    const litl::GraphicsPipelineOperations VulkanGraphicsPipelineOperations = {
        &build,
        &destroy
    };
}

#endif