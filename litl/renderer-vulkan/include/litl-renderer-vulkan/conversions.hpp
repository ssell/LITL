#ifndef LITL_VULKAN_RENDERER_CONVERSIONS_H__
#define LITL_VULKAN_RENDERER_CONVERSIONS_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/shaderEnums.hpp"

namespace litl::vulkan
{
    [[nodiscard]] VkShaderStageFlags toVkShaderStageFlags(ShaderStage stages) noexcept;
    [[nodiscard]] ShaderStage fromVkShaderStageFlags(VkShaderStageFlags flags) noexcept;

    [[nodiscard]] VkDescriptorType toVkDescriptorType(ShaderResourceType type) noexcept;
    [[nodiscard]] ShaderResourceType fromVkDescriptorType(VkDescriptorType type) noexcept;
}

#endif