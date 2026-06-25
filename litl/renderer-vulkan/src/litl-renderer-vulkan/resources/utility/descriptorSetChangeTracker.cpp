#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/math/bit.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-renderer-vulkan/resources/pipeline.hpp"

namespace litl::vulkan
{
    void DescriptorSetChangeTracker::addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorBufferInfo bufferInfo) noexcept
    {
        addChange(DescriptorSetChange{
            .binding = binding,
            .type = type,
            .isBuffer = true,
            .bufferInfo = bufferInfo
        }, set);
    }

    void DescriptorSetChangeTracker::addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorImageInfo imageInfo) noexcept
    {
        addChange(DescriptorSetChange{
            .binding = binding,
            .type = type,
            .isBuffer = false,
            .imageInfo = imageInfo
        }, set);
    }

    void DescriptorSetChangeTracker::addChange(DescriptorSetChange change, uint32_t set) noexcept
    {
        LITL_ASSERT_MSG((set < m_changes.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for buffer change.", );

        auto& changes = m_changes[set];
        auto bindingIndex = findBindingIndex(change.binding, changes);

        if (bindingIndex.has_value())
        {
            // Replace the change already bound to this index
            changes[bindingIndex.value()] = change;
        }
        else
        {
            // No existing binding, add it
            changes.push_back(change);
        }

        bitSet(m_dirtyMask, set);
    }

    void DescriptorSetChangeTracker::flushChanges(RendererContext& context, VkCommandBuffer vkCommandBuffer, PipelineResource& pipeline, bool isGraphics) noexcept
    {
        if (m_dirtyMask == 0u)
        {
            return;
        }

        const VkPipelineBindPoint vkBindPoint = (isGraphics ? VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS : VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE);

        for (uint32_t i = 0u; i < MaxDescriptorSets; ++i)
        {
            if (bitCheck(m_dirtyMask, i))
            {
                flushChange(context, vkCommandBuffer, pipeline.vkPipelineLayout, vkBindPoint, pipeline.setLayouts[i], i);
            }
        }

        m_dirtyMask = 0u;
    }

    [[nodiscard]] std::optional<uint32_t> DescriptorSetChangeTracker::findBindingIndex(uint32_t binding, std::vector<DescriptorSetChange>& changes) const noexcept
    {
        for (uint32_t i = 0u; i < static_cast<uint32_t>(changes.size()); ++i)
        {
            if (changes[i].binding == binding)
            {
                return i;
            }
        }

        return std::nullopt;
    }

    void DescriptorSetChangeTracker::flushChange(
        RendererContext& context, 
        VkCommandBuffer vkCommandBuffer, 
        VkPipelineLayout vkPipelineLayout, 
        VkPipelineBindPoint vkBindPoint, 
        VkDescriptorSetLayout vkDescriptorSetLayout, 
        uint32_t set) noexcept
    {
        auto& changes = m_changes[set];

        std::vector<VkWriteDescriptorSet> writes;
        writes.reserve(changes.size());

        for (auto& change : changes)
        {
            writes.push_back(VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = change.binding,
                .descriptorCount = 1,
                .descriptorType = change.type,
                .pBufferInfo = (change.isBuffer ? &change.bufferInfo : VK_NULL_HANDLE),
                .pImageInfo = (change.isBuffer ? VK_NULL_HANDLE : &change.imageInfo)
            });
        }

        // PerObject is the dedicated "push" set. There can only be one push set.
        if (set == static_cast<uint32_t>(DescriptorSetIndex::PerObject))
        {
            vkCmdPushDescriptorSet(
                vkCommandBuffer,
                vkBindPoint,
                vkPipelineLayout,
                set,
                static_cast<uint32_t>(writes.size()),
                writes.data()
            );
        }
        else
        {
            VkDescriptorSet vkDescriptorSet = context.getCurrFrameSyncInfo().descriptorSetAllocator->allocate(vkDescriptorSetLayout);

            for (auto& write : writes)
            {
                // Push path does not specify a dstSet, so set it now
                write.dstSet = vkDescriptorSet;
            }

            vkUpdateDescriptorSets(
                context.device.vkDevice,
                static_cast<uint32_t>(writes.size()),
                writes.data(),
                0,
                nullptr);

            vkCmdBindDescriptorSets(
                vkCommandBuffer,
                vkBindPoint,
                vkPipelineLayout,
                set,
                1,
                &vkDescriptorSet,
                0,
                nullptr);
        }
    }
}