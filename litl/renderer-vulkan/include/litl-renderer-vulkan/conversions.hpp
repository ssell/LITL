#ifndef LITL_VULKAN_RENDERER_CONVERSIONS_H__
#define LITL_VULKAN_RENDERER_CONVERSIONS_H__

#include <vulkan/vulkan.h>

#include "litl-renderer/enums.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl::vulkan
{
    [[nodiscard]] VkShaderStageFlags toVkShaderStageFlags(ShaderStage stages) noexcept;
    [[nodiscard]] ShaderStage fromVkShaderStageFlags(VkShaderStageFlags flags) noexcept;

    [[nodiscard]] VkDescriptorType toVkDescriptorType(ShaderResourceType type) noexcept;
    [[nodiscard]] ShaderResourceType fromVkDescriptorType(VkDescriptorType type) noexcept;

    [[nodiscard]] VkFormat toVkFormat(ImageFormat format) noexcept;
    [[nodiscard]] ImageFormat fromVkFormat(VkFormat format) noexcept;
}

#endif