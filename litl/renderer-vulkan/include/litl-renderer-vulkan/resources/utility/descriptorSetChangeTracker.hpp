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
    struct GraphicsPipelineResource;

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

        struct DescriptorSet
        {
            std::vector<DescriptorSetChange> changes;
        };

    public:

        static constexpr uint32_t MaxDescriptorSets = 32u;
        static constexpr uint32_t MaxBindingsPerSet = 32u;
        static_assert(MaxDescriptorSets <= 32u, "MaxDescriptorSets must be <= 32 to match the number of bits available in the dirty mask.");

        DescriptorSetChangeTracker();

        /// <summary>
        /// Resets the change tracker clearing away tracked changes.
        /// Should be called when the owning command buffer is reset (new frame, etc.).
        /// </summary>
        void reset() noexcept;

        /// <summary>
        /// Adds a change to the binding of a buffer resource.
        /// </summary>
        /// <param name="binding"></param>
        /// <param name="set"></param>
        /// <param name="type"></param>
        /// <param name="bufferInfo"></param>
        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorBufferInfo bufferInfo) noexcept;

        /// <summary>
        /// Adds a change to the binding of a image/sampler resource.
        /// </summary>
        /// <param name="binding"></param>
        /// <param name="set"></param>
        /// <param name="type"></param>
        /// <param name="imageInfo"></param>
        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorImageInfo imageInfo) noexcept;

        /// <summary>
        /// Applies all pending descriptor set changes.
        /// Should be called immediately prior to a draw command (vkCmdDraw, etc.).
        /// </summary>
        /// <param name="context"></param>
        /// <param name="vkCommandBuffer"></param>
        /// <param name="pipeline"></param>
        /// <param name="isGraphics"></param>
        void flushChanges(RendererContext& context, VkCommandBuffer vkCommandBuffer, PipelineResource& pipeline, bool isGraphics) noexcept;

        /// <summary>
        /// Compares the compatibility between the newly bound pipeline and the previous pipeline.
        /// Dirties any bits in the tracking mask where the first set diverges onwards.
        /// </summary>
        /// <param name="prev"></param>
        /// <param name="curr"></param>
        void onPipelineLayoutChange(PipelineResource const* prev, PipelineResource const& curr) noexcept;

    private:

        void addChange(DescriptorSetChange change, uint32_t set) noexcept;
        [[nodiscard]] std::optional<uint32_t> findBindingIndex(uint32_t binding, std::vector<DescriptorSetChange>& changes) const noexcept;
        void flushChange(RendererContext& context, VkCommandBuffer vkCommandBuffer, VkPipelineLayout vkPipelineLayout, VkPipelineBindPoint vkBindPoint, VkDescriptorSetLayout vkDescriptorSetLayout, uint32_t set) noexcept;

        uint32_t m_dirtyMask = 0u;
        std::array<DescriptorSet, MaxDescriptorSets> m_descriptorSets;

    };
}

#endif