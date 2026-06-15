#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-core/hash.hpp"

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

    uint32_t getMaxPushConstantSize(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        VkPhysicalDeviceProperties physicalProperties;
        vkGetPhysicalDeviceProperties(vulkanContext->device.vkPhysicalDevice, &physicalProperties);

        return physicalProperties.limits.maxPushConstantsSize;
    }

    PipelineResourceKey getPipelineResourceKey(litl::RendererContext* context, std::string_view name) noexcept
    {
        return fastHashString(name);
    }
}