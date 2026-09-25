#ifndef LITL_RENDERER_VULKAN_REQUIRED_FEATURES_H__
#define LITL_RENDERER_VULKAN_REQUIRED_FEATURES_H__

#include "litl-renderer-vulkan/common.hpp"

namespace litl
{
    struct RequiredFeatureChain
    {
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT vulkanDynamicStateFeatures{};
        VkPhysicalDeviceVulkan14Features vulkan14Features{};
        VkPhysicalDeviceVulkan13Features vulkan13Features{};
        VkPhysicalDeviceVulkan12Features vulkan12Features{};
        VkPhysicalDeviceVulkan11Features vulkan11Features{};
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{};
    };

    [[nodiscard]] bool doesPhysicalDeviceSupportRequiredFeatures(VkPhysicalDevice device) noexcept;
    [[nodiscard]] RequiredFeatureChain createRequiredFeaturesChain() noexcept;
}

#endif