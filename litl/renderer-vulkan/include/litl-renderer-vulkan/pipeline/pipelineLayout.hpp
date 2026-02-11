#ifndef LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    struct PipelineLayoutHandle
    {
        VkDevice vkDevice;
        VkPipelineLayout vkPipelineLayout;
    };

    /// <summary>
    /// Instantiates a new PipelineLayout and returns the raw owning pointer to it.
    /// </summary>
    /// <param name="vkDevice"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    LITL::Renderer::PipelineLayout* createPipelineLayout(VkDevice vkDevice, LITL::Renderer::PipelineLayoutDescriptor const& descriptor);

    bool build(LITL::Renderer::PipelineLayoutDescriptor const& descriptor, LITL::Renderer::PipelineLayoutHandle const& litlHandle);
    void destroy(LITL::Renderer::PipelineLayoutHandle const& litlHandle);

    const LITL::Renderer::PipelineLayoutOperations VulkanPipelineLayoutOperations = {
        &build,
        &destroy
    };
}

#endif