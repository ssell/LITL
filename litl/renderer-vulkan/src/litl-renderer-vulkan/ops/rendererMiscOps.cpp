#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"

namespace litl::vulkan
{
    DataFormat getSwapchainImageFormat(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return fromVkFormat(vulkanContext->swapChain.vkSwapChainImageFormat);
    }

    FrameData getFrameData(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->renderInfo.frame;
    }
}