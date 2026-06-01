#ifndef LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__
#define LITL_RENDERER_VULKAN_GRAPHICS_PIPELINE_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"

namespace litl::vulkan
{
    struct GraphicsPipelineResource
    {
        VkPipeline vkPipeline = VK_NULL_HANDLE;
    };
}

#endif