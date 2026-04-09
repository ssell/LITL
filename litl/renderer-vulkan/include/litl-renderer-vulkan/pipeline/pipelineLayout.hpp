#ifndef LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace litl::vulkan
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
    litl::PipelineLayout* createPipelineLayout(VkDevice vkDevice, litl::PipelineLayoutDescriptor const& descriptor);

    bool build(litl::PipelineLayoutDescriptor const& descriptor, litl::PipelineLayoutHandle const& litlHandle);
    void destroy(litl::PipelineLayoutHandle const& litlHandle);

    const litl::PipelineLayoutOperations VulkanPipelineLayoutOperations = {
        &build,
        &destroy
    };
}

#endif