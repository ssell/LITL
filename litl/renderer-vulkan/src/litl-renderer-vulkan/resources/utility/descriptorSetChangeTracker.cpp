#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/math/bit.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    void DescriptorSetChangeTracker::addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorBufferInfo bufferInfo) noexcept
    {
        LITL_ASSERT_MSG((set < m_pendingChanges.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for buffer change.", );
        
        m_pendingChanges[set].push_back(DescriptorSetChange{
            .binding = binding,
            .type = type,
            .isBuffer = true,
            .bufferInfo = bufferInfo
        });

        bitSet(m_dirtyMask, set);
    }

    void DescriptorSetChangeTracker::addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorImageInfo imageInfo) noexcept
    {
        LITL_ASSERT_MSG((set < m_pendingChanges.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for image/sampler change.", );
        
        m_pendingChanges[set].push_back(DescriptorSetChange{
            .binding = binding,
            .type = type,
            .isBuffer = false,
            .imageInfo = imageInfo
        });

        bitSet(m_dirtyMask, set);
    }

    void DescriptorSetChangeTracker::flushChanges(
        RendererContext& context, 
        VkCommandBuffer vkCommandBuffer, 
        VkPipelineLayout vkPipelineLayout, 
        VkPipelineBindPoint vkBindPoint, 
        VkDescriptorSetLayout vkDescriptorSetLayout, 
        uint32_t set) noexcept
    {
        LITL_ASSERT_MSG((set < m_pendingChanges.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for image/sampler change.", );
        
        if (bitCheck(m_dirtyMask, set))
        {
            auto const& pending = m_pendingChanges[set];

            std::vector<VkWriteDescriptorSet> writes;
            writes.reserve(pending.size());

            for (auto& change : pending)
            {
                writes.push_back(VkWriteDescriptorSet{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstBinding = change.binding,
                    .descriptorCount = 1,
                    .descriptorType = change.type,
                    .pBufferInfo = &change.bufferInfo
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

            m_pendingChanges[set].clear();
            bitClear(m_dirtyMask, set);
        }
    }
}