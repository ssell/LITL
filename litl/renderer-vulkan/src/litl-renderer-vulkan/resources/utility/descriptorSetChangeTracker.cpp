#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/math/bit.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"
#include "litl-renderer-vulkan/resources/pipeline.hpp"

namespace litl::vulkan
{
    DescriptorSetChangeTracker::DescriptorSetChangeTracker()
    {
        for (auto& descriptorSet : m_descriptorSets)
        {
            descriptorSet.changes.reserve(MaxBindingsPerSet);
        }
    }

    void DescriptorSetChangeTracker::reset() noexcept
    {
        m_dirtyMask = 0u;

        for (auto& set : m_descriptorSets)
        {
            set.changes.clear();
        }
    }

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
        LITL_ASSERT_MSG((set < m_descriptorSets.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for buffer change.", );

        auto& changes = m_descriptorSets[set].changes;
        auto bindingIndex = findBindingIndex(change.binding, changes);

        if (bindingIndex.has_value())
        {
            // Replace the change already bound to this index
            changes[bindingIndex.value()] = change;
        }
        else
        {
            // No existing binding, add it
            LITL_ASSERT_MSG((changes.size() < static_cast<size_t>(MaxBindingsPerSet)), "DescriptorSetChangeTracker is full.", );
            changes.push_back(change);
        }

        bitSet(m_dirtyMask, set);
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

    void DescriptorSetChangeTracker::flushChanges(RendererContext& context, VkCommandBuffer vkCommandBuffer, PipelineResource& pipeline, bool isGraphics) noexcept
    {
        if (m_dirtyMask == 0u)
        {
            return;
        }

        const VkPipelineBindPoint vkBindPoint = (isGraphics ? VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS : VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE);

        for (uint32_t i = 0u; i < MaxDescriptorSets; ++i)
        {
            if (bitCheck(m_dirtyMask, i) && (i < pipeline.setLayouts.size()))
            {
                flushChange(context, vkCommandBuffer, pipeline.vkPipelineLayout, vkBindPoint, pipeline.setLayouts[i], i);
            }
        }

        m_dirtyMask = 0u;
    }

    void DescriptorSetChangeTracker::flushChange(
        RendererContext& context, 
        VkCommandBuffer vkCommandBuffer, 
        VkPipelineLayout vkPipelineLayout, 
        VkPipelineBindPoint vkBindPoint, 
        VkDescriptorSetLayout vkDescriptorSetLayout, 
        uint32_t set) noexcept
    {
        auto& changes = m_descriptorSets[set].changes;

        if (changes.size() == 0ull)
        {
            return;
        }

        std::array<VkWriteDescriptorSet, MaxBindingsPerSet> writes;

        for (uint32_t i = 0u; (i < MaxBindingsPerSet) && (i < changes.size()); ++i)
        {
            auto& change = changes[i];

            writes[i] = VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstBinding = change.binding,
                .descriptorCount = 1,
                .descriptorType = change.type,
                .pBufferInfo = (change.isBuffer ? &change.bufferInfo : VK_NULL_HANDLE),
                .pImageInfo = (change.isBuffer ? VK_NULL_HANDLE : &change.imageInfo)
            };
        }

        const uint32_t writesCount = min(MaxBindingsPerSet, static_cast<uint32_t>(changes.size()));

        // PerObject is the dedicated "push" set. There can only be one push set.
        if (set == static_cast<uint32_t>(DescriptorSetIndex::PerObject))
        {
            vkCmdPushDescriptorSet(
                vkCommandBuffer,
                vkBindPoint,
                vkPipelineLayout,
                set,
                writesCount,
                writes.data()
            );
        }
        else
        {
            VkDescriptorSet vkDescriptorSet = context.getCurrFrameSyncInfo().descriptorSetAllocator->allocate(vkDescriptorSetLayout);

            for (auto i = 0u; i < writesCount; ++i)
            {
                // Push path does not specify a dstSet, so set it now
                writes[i].dstSet = vkDescriptorSet;
            }

            vkUpdateDescriptorSets(
                context.device.vkDevice,
                writesCount,
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

    void DescriptorSetChangeTracker::onPipelineLayoutChange(PipelineResource const* prev, PipelineResource const& curr) noexcept
    {
        if (prev == nullptr)
        {
            // The first bind of the frame/command buffer. Everything needs to be rebound.
            m_dirtyMask = ~0u;
            return;
        }

        if (prev->vkPipelineLayout == curr.vkPipelineLayout)
        {
            // Pipeline layouts are the same so current bindings are fully compatible.
            return;
        }

        // Different layouts. Determine the first set where they diverge.
        uint32_t firstDirty = MaxDescriptorSets;

        for (uint32_t i = 0u; i < MaxDescriptorSets; ++i)
        {
            VkDescriptorSetLayout prevLayout = (i < prev->setLayouts.size()) ? prev->setLayouts[i] : VK_NULL_HANDLE;
            VkDescriptorSetLayout currLayout = (i < curr.setLayouts.size()) ? curr.setLayouts[i] : VK_NULL_HANDLE;

            if (prevLayout != currLayout)
            {
                firstDirty = i;
                break;
            }
        }

        // Cascade the dirty bits from the firstDirty onwards.
        for (uint32_t i = firstDirty; i < MaxDescriptorSets; ++i)
        {
            bitSet(m_dirtyMask, i);
        }

        /**
         * Note to future self:
         * Compute and graphics bind points are independent in Vulkan. If you ever interleave compute and graphics binds, 
         * you'd want a parallel onComputePipelineLayoutChange or a unified one parameterized by bind point. Same shape, just two state slots on the tracker.
         */
    }
}