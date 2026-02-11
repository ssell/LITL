#ifndef LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__
#define LITL_VULKAN_RENDERER_GRAPHICS_PIPELINE_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/graphicsPipeline.hpp"

namespace LITL::Vulkan::Renderer
{
    struct GraphicsPipelineHandle
    {
        VkDevice vkDevice;
        VkFormat vkSwapChainImageFormat;
        VkPipeline vkPipeline;
    };

    LITL::Renderer::GraphicsPipeline* createGraphicsPipeline(VkDevice vkDevice, VkFormat vkSwapChainImageFormat, LITL::Renderer::GraphicsPipelineDescriptor const& descriptor);

    bool build(LITL::Renderer::GraphicsPipelineDescriptor const& descriptor, LITL::Renderer::GraphicsPipelineHandle const& litlHandle);
    void destroy(LITL::Renderer::GraphicsPipelineHandle const& litlHandle);

    const LITL::Renderer::GraphicsPipelineOperations VulkanGraphicsPipelineOperations = {
        &build,
        &destroy
    };
}

#endif