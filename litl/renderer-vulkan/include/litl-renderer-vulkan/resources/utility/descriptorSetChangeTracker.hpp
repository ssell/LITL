#ifndef LITL_RENDERER_VULKAN_DESCRIPTOR_SET_CHANGE_TRACKER_H__
#define LITL_RENDERER_VULKAN_DESCRIPTOR_SET_CHANGE_TRACKER_H__

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
    struct RendererContext;
    struct PipelineResource;

    class DescriptorSetChangeTracker final
    {
        /// <summary>
        /// A single pending descriptor set change that needs to be updated and bound.
        /// </summary>
        struct DescriptorSetChange
        {
            uint32_t binding;
            VkDescriptorType type;
            bool isBuffer;

            union
            {
                VkDescriptorBufferInfo bufferInfo;
                VkDescriptorImageInfo imageInfo;
            };
        };

    public:

        static constexpr uint32_t MaxDescriptorSets = 32u;

        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorBufferInfo bufferInfo) noexcept;
        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorImageInfo imageInfo) noexcept;
        void flushChanges(RendererContext& context, VkCommandBuffer vkCommandBuffer, PipelineResource& pipeline, bool isGraphics) noexcept;

    private:

        void addChange(DescriptorSetChange change, uint32_t set) noexcept;
        [[nodiscard]] std::optional<uint32_t> findBindingIndex(uint32_t binding, std::vector<DescriptorSetChange>& changes) const noexcept;
        void flushChange(RendererContext& context, VkCommandBuffer vkCommandBuffer, VkPipelineLayout vkPipelineLayout, VkPipelineBindPoint vkBindPoint, VkDescriptorSetLayout vkDescriptorSetLayout, uint32_t set) noexcept;

        uint32_t m_dirtyMask = 0u;
        std::array<std::vector<DescriptorSetChange>, MaxDescriptorSets> m_changes;

    };
}

#endif