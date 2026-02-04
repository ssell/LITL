#ifndef LITL_VULKAN_RENDERER_SWAP_CHAIN_SUPPORT_H__
#define LITL_VULKAN_RENDERER_SWAP_CHAIN_SUPPORT_H__

#include <vector>
#include <vulkan/vulkan.h>

namespace LITL::Vulkan::Renderer
{
    struct SwapChainSupport
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        static SwapChainSupport querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool isSwapChainSufficient() const noexcept;
        VkSurfaceFormatKHR chooseSwapSurfaceFormat() const noexcept;
        VkPresentModeKHR chooseSwapPresentMode() const noexcept;
        VkExtent2D chooseSwapExtent(int32_t width, int32_t height) const noexcept;
    };
}

#endif