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

    [[nodiscard]] VkFormat toVkFormat(DataFormat format) noexcept;
    [[nodiscard]] DataFormat fromVkFormat(VkFormat format) noexcept;

    [[nodiscard]] VkImageLayout toVkImageLayout(ImageLayoutType layout) noexcept;
    [[nodiscard]] ImageLayoutType fromVkImageLayout(VkImageLayout layout) noexcept;

    [[nodiscard]] VkAccessFlags2 toVkAccessFlag(ImageAccessFlag flag) noexcept;
    [[nodiscard]] ImageAccessFlag fromVkAccessFlag(VkAccessFlags2 flag) noexcept;

    [[nodiscard]] VkPipelineStageFlags2 toVkPipelineStageFlag(PipelineStageFlag flag) noexcept;
    [[nodiscard]] PipelineStageFlag fromVkPipelineStageFlag(VkPipelineStageFlags2 flag) noexcept;

    [[nodiscard]] VkAttachmentLoadOp toVkAttachmentLoadOp(LoadOperationType op) noexcept;
    [[nodiscard]] LoadOperationType fromVkAttachmentLoadOp(VkAttachmentLoadOp op) noexcept;

    [[nodiscard]] VkAttachmentStoreOp toVkAttachmentStoreOp(StoreOperationType op) noexcept;
    [[nodiscard]] StoreOperationType fromVkAttachmentStoreOp(VkAttachmentStoreOp op) noexcept;

    [[nodiscard]] VkPrimitiveTopology toVkPrimitiveTopology(PrimitiveTopology topology) noexcept;
    [[nodiscard]] PrimitiveTopology fromVkPrimitiveTopology(VkPrimitiveTopology topology) noexcept;

    [[nodiscard]] VkPolygonMode toVkPolygonMode(PolygonMode mode) noexcept;
    [[nodiscard]] PolygonMode fromVkPolygonMode(VkPolygonMode mode) noexcept;

    [[nodiscard]] VkCullModeFlags toVkCullModeFlag(CullMode mode) noexcept;
    [[nodiscard]] CullMode fromVkCullModeFlag(VkCullModeFlags mode) noexcept;

    [[nodiscard]] VkFrontFace toVkFrontFace(FrontFace face) noexcept;
    [[nodiscard]] FrontFace fromVkFrontFace(VkFrontFace face) noexcept;

    [[nodiscard]] VkSampleCountFlags toVkSampleCountFlag(MultisampleCount count) noexcept;
    [[nodiscard]] MultisampleCount fromVkSampleCountFlag(VkSampleCountFlags count) noexcept;

    [[nodiscard]] VkCompareOp toVkCompareOp(CompareOperationType op) noexcept;
    [[nodiscard]] CompareOperationType fromVkCompareOp(CompareOperationType op) noexcept;

    [[nodiscard]] VkStencilOp toVkStencilOp(StencilOperationType op) noexcept;
    [[nodiscard]] StencilOperationType fromVkStencilOp(VkStencilOp op) noexcept;

    [[nodiscard]] VkLogicOp toVkLogicOp(LogicOperationType op) noexcept;
    [[nodiscard]] LogicOperationType fromVkLogicOp(VkLogicOp op) noexcept;
}

#endif