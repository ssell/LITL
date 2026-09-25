#include "litl-renderer-vulkan/requiredFeatures.hpp"

namespace litl::vulkan
{
    [[nodiscard]] bool doesPhysicalDeviceSupportRequiredFeatures(VkPhysicalDevice device) noexcept
    {
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT extendedDynamicStateFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT, .pNext = nullptr };
        VkPhysicalDeviceVulkan14Features vulkan14Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = &extendedDynamicStateFeatures };
        VkPhysicalDeviceVulkan13Features vulkan13Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &vulkan14Features };
        VkPhysicalDeviceVulkan12Features vulkan12Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &vulkan13Features };
        VkPhysicalDeviceVulkan11Features vulkan11Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, .pNext = &vulkan12Features };
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &vulkan11Features };

        VkPhysicalDeviceFeatures2 supportedFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &physicalDeviceFeatures
        };

        vkGetPhysicalDeviceFeatures2(device, &supportedFeatures);

        if (!extendedDynamicStateFeatures.extendedDynamicState ||
            !vulkan14Features.pushDescriptor ||
            !vulkan13Features.synchronization2 ||
            !vulkan13Features.dynamicRendering ||
            !vulkan12Features.shaderSampledImageArrayNonUniformIndexing ||
            !vulkan12Features.descriptorBindingSampledImageUpdateAfterBind ||
            !vulkan12Features.descriptorBindingPartiallyBound ||
            !vulkan12Features.runtimeDescriptorArray ||
            !vulkan12Features.bufferDeviceAddress ||
            !vulkan11Features.shaderDrawParameters ||
            !physicalDeviceFeatures.features.geometryShader ||
            !physicalDeviceFeatures.features.tessellationShader ||
            !physicalDeviceFeatures.features.shaderInt64 ||
            !physicalDeviceFeatures.features.textureCompressionBC)
        {
            return false;
        }

        return true;
    }

    [[nodiscard]] RequiredFeatureChain createRequiredFeaturesChain() noexcept
    {
        RequiredFeatureChain chain{};

        chain.vulkanDynamicStateFeatures = VkPhysicalDeviceExtendedDynamicStateFeaturesEXT{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = nullptr,
            .extendedDynamicState = VK_TRUE                                 // Allows us to specify pipeline states during command buffer recording instead of having to bake Pipeline State Objects (PSOs)
        };

        chain.vulkan14Features = VkPhysicalDeviceVulkan14Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES,
            .pNext = &chain.vulkanDynamicStateFeatures,
            .pushDescriptor = VK_TRUE
        };

        chain.vulkan13Features = VkPhysicalDeviceVulkan13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &chain.vulkan14Features,
            .synchronization2 = VK_TRUE,                                    // Replaces the legacy Synchronization API with a cleaner easier-to-use design.
            .dynamicRendering = VK_TRUE,                                    // Removes the need to create explicit VkRenderPass and VkFramebuffer objects allowing us to begin and end rendering directly on image views.
        };

        chain.vulkan12Features = VkPhysicalDeviceVulkan12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &chain.vulkan13Features,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,           // Allows non-uniform indexing (per-vertex or per-pixel, not per-warp) of sampled image arrays. Needed for our bindless rendering and using a single large texture array.
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,        // Allows us to stream/update descriptors while the command buffer runs (edit our single large texture array) as long as indexes being samples are untouched.
            .descriptorBindingPartiallyBound = VK_TRUE,                     // Allows us to use descriptor bindings where some array elements or descriptors are not valid or populated, as long as those are never accessed during execution.
            .runtimeDescriptorArray = VK_TRUE,                              // Enables SPIR-V capability of the same name, which allows us to have unbounded/runtime-sized descriptor arrays.
            .bufferDeviceAddress = VK_TRUE                                  // Allows us to access buffer memory directly via 64-bit virtual memory pointers and removes the need for traditional descriptor sets. 
        };

        chain.vulkan11Features = VkPhysicalDeviceVulkan11Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &chain.vulkan12Features,
            .shaderDrawParameters = VK_TRUE                                 // Enables accessto built-in GLSL shader variables like gl_BaseVertex, gl_BaseInstance, and gl_DrawID.
        };

        chain.physicalDeviceFeatures = VkPhysicalDeviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &chain.vulkan11Features,
            .features = VkPhysicalDeviceFeatures {
                .geometryShader = VK_TRUE,                                  // Enables geometry shader support.
                .tessellationShader = VK_TRUE,                              // Enables tessellation shader suport.
                .textureCompressionBC = VK_TRUE,                            // Enable Block Compression (BC) texture formats. We make use of BC4, BC5, BC6H, and BC7.
                .shaderInt64 = VK_TRUE                                      // Add support for 64-bit signed and unsigned integers. Needed for BDA addresses.
            }
        };

        return chain;
    }
}