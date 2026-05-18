#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-renderer/renderer.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    bool build(litl::RendererContext* context) noexcept;
    void destroy(litl::RendererContext* context) noexcept;

    inline constexpr litl::RendererOps VulkanRendererOps = {
        &build,
        &destroy
    };
}

#endif 