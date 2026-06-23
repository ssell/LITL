#ifndef LITL_VULKAN_RENDERER_CONVERSIONS_H__
#define LITL_VULKAN_RENDERER_CONVERSIONS_H__

#include "litl-core/math/types.hpp"
#include "litl-renderer-vulkan/common.hpp"
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
    [[nodiscard]] CompareOperationType fromVkCompareOp(VkCompareOp op) noexcept;

    [[nodiscard]] VkStencilOp toVkStencilOp(StencilOperationType op) noexcept;
    [[nodiscard]] StencilOperationType fromVkStencilOp(VkStencilOp op) noexcept;

    [[nodiscard]] VkLogicOp toVkLogicOp(LogicOperationType op) noexcept;
    [[nodiscard]] LogicOperationType fromVkLogicOp(VkLogicOp op) noexcept;

    [[nodiscard]] VkBlendOp toVkBlendOp(BlendOperationType op) noexcept;
    [[nodiscard]] BlendOperationType fromVkBlendOp(VkBlendOp op) noexcept;

    [[nodiscard]] VkBlendFactor toVkBlendFactor(BlendFactor factor) noexcept;
    [[nodiscard]] BlendFactor fromVkBlendFactor(VkBlendFactor factor) noexcept;

    [[nodiscard]] VkColorComponentFlags toVkColorComponentFlag(ColorComponentFlag flag) noexcept;
    [[nodiscard]] ColorComponentFlag fromVkColorComponentFlag(VkColorComponentFlags flag) noexcept;

    [[nodiscard]] VkDynamicState toVkDynamicState(DynamicState state) noexcept;
    [[nodiscard]] DynamicState fromVkDynamicState(VkDynamicState state) noexcept;

    [[nodiscard]] VkVertexInputRate toVkVertexInputRate(VertexInputRate rate) noexcept;
    [[nodiscard]] VertexInputRate fromVkVertexInputRate(VkVertexInputRate rate) noexcept;

    [[nodiscard]] VkBufferUsageFlags2 toVkBufferUsageFlag(BufferTypeFlag flag) noexcept;
    [[nodiscard]] BufferTypeFlag fromVkBufferUsageFlag(VkBufferUsageFlags2 flag) noexcept;

    [[nodiscard]] VmaMemoryUsage toVmaMemoryUsage(BufferMemoryType usage) noexcept;
    [[nodiscard]] BufferMemoryType fromVmaMemoryUsage(VmaMemoryUsage usage) noexcept;
    [[nodiscard]] VmaAllocationCreateFlags toVmaAllocationCreateFlag(BufferMemoryUsage usage) noexcept;

    [[nodiscard]] VkSharingMode toVkSharingMode(SharingMode mode) noexcept;
    [[nodiscard]] SharingMode fromVkSharingMode(VkSharingMode mode) noexcept;

    [[nodiscard]] VkPipelineStageFlags2 deriveDstStageFromBufferType(BufferTypeFlag flag) noexcept;
    [[nodiscard]] VkAccessFlags2 deriveDstAccessFromBufferType(BufferTypeFlag flag) noexcept;

    [[nodiscard]] VkPipelineStageFlags2 deriveDstStageFromUsageFlag(VkBufferUsageFlags2 usage) noexcept;
    [[nodiscard]] VkAccessFlags2 deriveDstAccessFromUsageFlag(VkBufferUsageFlags2 usage) noexcept;

    [[nodiscard]] VkClearColorValue toVkClearColorValue(color color) noexcept;

    [[nodiscard]] VkIndexType toVkIndexType(IndexType type) noexcept;
    [[nodiscard]] IndexType fromVkIndexType(VkIndexType type) noexcept;

    [[nodiscard]] VkImageType toVkImageType(TextureDimensions type) noexcept;
    [[nodiscard]] TextureDimensions fromVkImageType(VkImageType type) noexcept;

    [[nodiscard]] VkFilter toVkFilter(SamplerFilter filter) noexcept;
    [[nodiscard]] SamplerFilter fromVkFilter(VkFilter filter) noexcept;

    [[nodiscard]] VkSamplerMipmapMode toVkSamplerMipmapMode(SamplerMipFilter filter) noexcept;
    [[nodiscard]] SamplerMipFilter fromVkSamplerMipmapMode(VkSamplerMipmapMode filter) noexcept;

    [[nodiscard]] VkSamplerAddressMode toVkSamplerAddressMode(SamplerAddressMode mode) noexcept;
    [[nodiscard]] SamplerAddressMode fromVkSamplerAddressMode(VkSamplerAddressMode mode) noexcept;

    [[nodiscard]] VkBorderColor toVkBorderColor(SamplerBorderColor color) noexcept;
    [[nodiscard]] SamplerBorderColor fromVkBorderColor(VkBorderColor color) noexcept;


}

#endif