#ifndef LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__
#define LITL_VULKAN_RENDERER_PIPELINE_LAYOUT_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    struct PipelineLayoutData
    {
        VkDevice device;
    };

    struct PipelineLayoutHandle
    {
        VkDevice device;
        VkPipelineLayout pipelineLayout;
    };

    bool build(LITL::Renderer::PipelineLayoutData const& data, LITL::Renderer::PipelineLayoutDescriptor const& descriptor, LITL::Renderer::PipelineLayoutHandle& handle);
    void destroy(LITL::Renderer::PipelineLayoutHandle const& handle);
}

#endif