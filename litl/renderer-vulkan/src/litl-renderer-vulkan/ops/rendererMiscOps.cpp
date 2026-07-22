#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"

namespace litl::vulkan
{
    DataFormat getSwapchainImageFormat(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return fromVkFormat(vulkanContext->swapChain.vkSwapChainImageFormat);
    }

    SwapChainDimensions getSwapchainDimensions(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        return SwapChainDimensions{
            .width = vulkanContext->swapChain.vkSwapChainExtent.width,
            .height = vulkanContext->swapChain.vkSwapChainExtent.height,
            .aspectRatio = static_cast<float>(vulkanContext->swapChain.vkSwapChainExtent.width) / static_cast<float>(vulkanContext->swapChain.vkSwapChainExtent.height)
        };
    }

    FrameData getFrameData(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);
        return vulkanContext->renderInfo.frame;
    }

    uint32_t getMaxPushConstantSize(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        VkPhysicalDeviceProperties physicalProperties;
        vkGetPhysicalDeviceProperties(vulkanContext->device.vkPhysicalDevice, &physicalProperties);

        return physicalProperties.limits.maxPushConstantsSize;
    }
}