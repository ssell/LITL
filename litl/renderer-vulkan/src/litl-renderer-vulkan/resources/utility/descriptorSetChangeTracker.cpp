#include "litl-renderer-vulkan/resources/utility/descriptorSetChangeTracker.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/math/bit.hpp"

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

    void DescriptorSetChangeTracker::flushChanges(uint32_t set) noexcept
    {
        LITL_ASSERT_MSG((set < m_pendingChanges.size()), "DescriptorSetChangeTracker provided out-of-bounds set index for image/sampler change.", );
        
        if (bitCheck(m_dirtyMask, set))
        {
            // ... todo ...
            bitClear(m_dirtyMask, set);
        }
    }
}