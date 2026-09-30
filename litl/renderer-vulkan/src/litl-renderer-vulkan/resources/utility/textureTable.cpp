#include <algorithm>
#include <array>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/resources/utility/textureTable.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    bool TextureTable::build(RendererContext& context) noexcept
    {
        m_pContext = &context;
        m_capacity = m_pContext->device.textureTableCapacity;
        m_freeCount = m_capacity;
        m_slotOwners.resize(m_capacity, {});

        if (!buildDescriptorPool() ||
            !buildDescriptorSetLayout() ||
            !buildDescriptorSet() ||
            !buildPipelineLayout())
        {
            return false;
        }

        // Reserve the first N indices for internal engine usage.
        m_head = static_cast<uint32_t>(TextureTableReservedIndices::ReservedIndicesCount);
        m_freeCount -= m_head;

        return true;
    }

    bool TextureTable::buildDescriptorPool() noexcept
    {
        const std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = m_capacity
            },
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount)
            }
        };

        const VkDescriptorPoolCreateInfo createDescriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1u,                                                              // Only one object being allocated from this pool
            .poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()), 
            .pPoolSizes = descriptorPoolSizes.data()
        };

        m_vkDescriptorPool = VK_NULL_HANDLE;
        const VkResult result = vkCreateDescriptorPool(m_pContext->device.vkDevice, &createDescriptorPoolInfo, nullptr, &m_vkDescriptorPool);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorPool for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    bool TextureTable::buildDescriptorSetLayout() noexcept
    {
        const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
            VkDescriptorSetLayoutBinding {
                .binding = 0u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = m_capacity,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding {
                .binding = 1u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(m_pContext->samplerArray.getVkSamplers().size()),
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = m_pContext->samplerArray.getVkSamplers().data()
            }
        };

        const std::array< VkDescriptorBindingFlags, 2> bindingFlags = {
            VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT },
            VkDescriptorBindingFlags { 0 }
        };

        LITL_ASSERT_MSG(bindings.size() == bindingFlags.size(), "Mismatch between bindings count and binding flags count in Vulkan Texture Table", false);

        const VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .pNext = nullptr,
            .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data()
        };

        const VkDescriptorSetLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = &bindingFlagsInfo,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindings = bindings.data()
        };

        m_vkDescriptorSetLayout = VK_NULL_HANDLE;
        const VkResult result = vkCreateDescriptorSetLayout(m_pContext->device.vkDevice, &createInfo, nullptr, &m_vkDescriptorSetLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorSetLayout for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    bool TextureTable::buildDescriptorSet() noexcept
    {
        const VkDescriptorSetAllocateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_vkDescriptorPool,
            .descriptorSetCount = 1u,
            .pSetLayouts = &m_vkDescriptorSetLayout
        };

        m_vkDescriptorSet = VK_NULL_HANDLE;
        const VkResult result = vkAllocateDescriptorSets(m_pContext->device.vkDevice, &createInfo, &m_vkDescriptorSet);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorSet for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    bool TextureTable::buildPipelineLayout() noexcept
    {
        // Note that this needs to stay in sync with the push constant range defined in the PipelineLayoutCache
        const VkPushConstantRange pushConstantRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0u,
            .size = 128u
        };

        const VkPipelineLayoutCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = 1u,
            .pSetLayouts = &m_vkDescriptorSetLayout,
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges = &pushConstantRange
        };

        const VkResult result = vkCreatePipelineLayout(m_pContext->device.vkDevice, &createInfo, nullptr, &m_vkPipelineLayout);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkPipelineLayout for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    void TextureTable::destroy() noexcept
    {
        m_slotOwners.clear();
        m_freeSlots.clear();
        m_head = 0u;
        m_capacity = 0u;
        m_freeCount = 0u;

        if (m_pContext == nullptr)
        {
            return;
        }

        if (m_pContext->device.vkDevice != VK_NULL_HANDLE)
        {
            if (m_vkDescriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(m_pContext->device.vkDevice, m_vkDescriptorSetLayout, nullptr);
                m_vkDescriptorSetLayout = VK_NULL_HANDLE;
            }

            if (m_vkDescriptorPool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_pContext->device.vkDevice, m_vkDescriptorPool, nullptr);
                m_vkDescriptorPool = VK_NULL_HANDLE;
            }

            if (m_vkPipelineLayout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(m_pContext->device.vkDevice, m_vkPipelineLayout, nullptr);
                m_vkPipelineLayout = VK_NULL_HANDLE;
            }
        }
    }

    uint32_t TextureTable::acquire(TextureResourceHandle handle) noexcept
    {
        uint32_t slot = Constants::uint32_null_index;

        if (!handle.isValid())
        {
            logWarning("Attempting to acquire slot in Vulkan TextureTable with invalid TextureResourceHandle.");
            return slot;
        }

        if (m_freeCount == 0u)
        {
            logWarning("Vulkan TextureTable is out of capacity.");
            return slot;
        }

        // First check the free list ...
        if (!m_freeSlots.empty())
        {
            slot = m_freeSlots.back();
            m_freeSlots.pop_back();
        }
        // Otherwise, take from the head
        else
        {
            slot = m_head;
            m_head++;
        }

        m_freeCount--;
        m_slotOwners[slot] = handle;

        if (!writeSlot(slot, m_pContext->resources.getTexture(handle)))
        {
            // Failed to write into the slot. Undo the slot allocation and return a null index.
            m_freeCount++;
            m_freeSlots.push_back(slot);
            m_slotOwners[slot] = {};

            return Constants::uint32_null_index;
        }

        return slot;
    }

    bool TextureTable::writeSlot(uint32_t slot, TextureResource* texture) noexcept
    {
        if (texture == nullptr)
        {
            return false;
        }

        const VkDescriptorImageInfo imageInfo{
            .sampler = VK_NULL_HANDLE,              // We have a separate sampler array and not a combined texture+sampler array.
            .imageView = (texture->vkSampledImageView != VK_NULL_HANDLE ? texture->vkSampledImageView : texture->vkImageView),
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        const VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = m_vkDescriptorSet,
            .dstBinding = 0u,
            .dstArrayElement = slot,
            .descriptorCount = 1u,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(m_pContext->device.vkDevice, 1u, &write, 0u, nullptr);

        return true;
    }

    bool TextureTable::release(uint32_t slot) noexcept
    {
        if (slot >= m_capacity)
        {
            logWarning("Attempting to free slot ", slot, " in Vulkan texture table that is out-of-bounds (max = ", m_capacity, ")");
            return false;
        }

        if (!m_slotOwners[slot].isValid())
        {
            logWarning("Attempting to free slot in Vulkan texture table that is not currently owned.");
            return false;
        }

        m_slotOwners[slot] = m_slotOwners[static_cast<uint32_t>(TextureTableReservedIndices::Pink)];        // A released texture will show up as pink if a bad reference exists
        m_freeSlots.push_back(slot);
        m_freeCount++;

        return true;
    }

    bool TextureTable::update(uint32_t slot, TextureResourceHandle handle) noexcept
    {
        if (!handle.isValid())
        {
            logWarning("Attempting to update slot ", slot, " in Vulkan texture table with an invalid handle.");
            return false;
        }

        if (slot >= m_capacity)
        {
            logWarning("Attempting to update slot ", slot, " in Vulkan texture table that is out-of-bounds (max = ", m_capacity, ")");
            return false;
        }

        if (m_slotOwners[slot].isValid())
        {
            // Updating an existing occupied slot. Capacity is already accounted for.
            logWarning("Updating an existing occupied Vulkan texture table slot (", slot, "). Release by previous owner may have an unintended result.");
            m_slotOwners[slot] = handle;
        }
        else
        {
            // Updating an unoccupied slot. This is typically avoided except for the engine reserved default textures.
            std::erase(m_freeSlots, slot);
            m_slotOwners[slot] = handle;
            m_freeCount--;
        }

        if (!writeSlot(slot, m_pContext->resources.getTexture(handle)))
        {
            m_freeCount++;
            m_freeSlots.push_back(slot);
            m_slotOwners[slot] = {};

            return false;
        }

        return true;
    }

    VkDescriptorSet TextureTable::getDescriptorSet() const noexcept
    {
        return m_vkDescriptorSet;
    }

    VkDescriptorSetLayout TextureTable::getDescriptorSetLayout() const noexcept
    {
        return m_vkDescriptorSetLayout;
    }

    VkPipelineLayout TextureTable::getPipelineLayout() const noexcept
    {
        return m_vkPipelineLayout;
    }
}