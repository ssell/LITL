#ifndef LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    std::unique_ptr<LITL::Renderer::PipelineLayout> createPipelineLayout(VkDevice vkDevice);

    bool build(LITL::Renderer::PipelineLayoutDescriptor const& descriptor, LITL::Renderer::PipelineLayoutHandle const& litlHandle);
    void destroy(LITL::Renderer::PipelineLayoutHandle const& litlHandle);

    const LITL::Renderer::PipelineLayoutOperations VulkanPipelineLayoutOperations = {
        &build,
        &destroy
    };
}

#endif