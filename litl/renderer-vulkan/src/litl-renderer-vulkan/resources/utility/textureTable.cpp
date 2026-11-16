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

        getSubTable(TextureSubTableType::Texture2D).capacity = m_pContext->device.texture2DTableCapacity;
        getSubTable(TextureSubTableType::Texture2DArray).capacity = m_pContext->device.texture2DArrayTableCapacity;
        getSubTable(TextureSubTableType::Texture2DCubeArray).capacity = m_pContext->device.texture2DCubeTableCapacity;

        for (auto& subtable : m_subtables)
        {
            subtable.freeCount = subtable.capacity;
            subtable.slotOwners.resize(subtable.capacity, {});
        }

        m_subtables[0].head = static_cast<uint32_t>(TextureTableReservedIndices::ReservedIndicesCount);
        m_subtables[0].freeCount -= m_subtables[0].head;

        if (!buildDescriptorPool() ||
            !buildDescriptorSetLayout() ||
            !buildDescriptorSet() ||
            !buildPipelineLayout())
        {
            return false;
        }

        return true;
    }

    bool TextureTable::buildDescriptorPool() noexcept
    {
        const std::array<VkDescriptorPoolSize, 4> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2D).capacity
            },
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2DArray).capacity
            },
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2DCubeArray).capacity
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
        const std::array<VkDescriptorSetLayoutBinding, 4> bindings = {
            VkDescriptorSetLayoutBinding {
                .binding = 0u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2D).capacity,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding {
                .binding = 1u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2DArray).capacity,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding {
                .binding = 2u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = getSubTable(TextureSubTableType::Texture2DCubeArray).capacity,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            },
            VkDescriptorSetLayoutBinding {
                .binding = 3u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(m_pContext->samplerArray.getVkSamplers().size()),
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = m_pContext->samplerArray.getVkSamplers().data()
            }
        };

        const std::array< VkDescriptorBindingFlags, 4> bindingFlags = {
            VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT },
            VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT },
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
        for (auto& subtable : m_subtables)
        {
            subtable.capacity = 0u;
            subtable.slotOwners.clear();
            subtable.freeSlots.clear();
            subtable.head = 0u;
            subtable.freeCount = 0u;
        }

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

    uint32_t TextureTable::acquire(TextureResourceHandle handle, bool isArray, bool isCube) noexcept
    {
        uint32_t slot = constants::uint32_null_index;

        if (!handle.isValid())
        {
            logWarning("Attempting to acquire slot in Vulkan TextureTable with invalid TextureResourceHandle.");
            return slot;
        }

        auto& subtable = getSubTable(getSubTableType(isArray, isCube));

        if (subtable.freeCount == 0u)
        {
            logWarning("Vulkan TextureTable is out of capacity.");
            return slot;
        }

        // First check the free list ...
        if (!subtable.freeSlots.empty())
        {
            slot = subtable.freeSlots.back();
            subtable.freeSlots.pop_back();
        }
        // Otherwise, take from the head
        else
        {
            slot = subtable.head;
            subtable.head++;
        }

        subtable.freeCount--;
        subtable.slotOwners[slot] = handle;

        if (!writeSlot(slot, m_pContext->resources.getTexture(handle)))
        {
            // Failed to write into the slot. Undo the slot allocation and return a null index.
            subtable.freeCount++;
            subtable.freeSlots.push_back(slot);
            subtable.slotOwners[slot] = {};

            return constants::uint32_null_index;
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
            .dstBinding = static_cast<uint32_t>(getSubTableType(texture->descriptor.isArray(), texture->descriptor.isCubeMap)),
            .dstArrayElement = slot,
            .descriptorCount = 1u,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageInfo
        };

        vkUpdateDescriptorSets(m_pContext->device.vkDevice, 1u, &write, 0u, nullptr);

        return true;
    }

    bool TextureTable::release(uint32_t slot, TextureResourceHandle handle, bool isArray, bool isCube) noexcept
    {
        auto& subtable = getSubTable(getSubTableType(isArray, isCube));

        if (slot >= subtable.capacity)
        {
            logWarning("Attempting to free slot ", slot, " in Vulkan texture table that is out-of-bounds (max = ", subtable.capacity, ")");
            return false;
        }

        if (subtable.slotOwners[slot] != handle)
        {
            return true;    // No longer in the slot.
        }

        if (!subtable.slotOwners[slot].isValid())
        {
            logWarning("Attempting to free slot in Vulkan texture table that is not currently owned.");
            return false;
        }

        subtable.slotOwners[slot] = {};
        subtable.freeSlots.push_back(slot);
        subtable.freeCount++;

        if (subtable.fallbackHandle.isValid() && !writeSlot(slot, m_pContext->resources.getTexture(subtable.fallbackHandle)))
        {
            logWarning("Failed to set released texture table slot ", slot, " to fallback texture handle.");
        }

        if (slot == 0u)
        {
            // The fallback handle was just released.
            subtable.fallbackHandle = {};
        }

        return true;
    }

    bool TextureTable::update(uint32_t slot, TextureResourceHandle handle, bool isArray, bool isCube) noexcept
    {
        if (!handle.isValid())
        {
            logWarning("Attempting to update slot ", slot, " in Vulkan texture table with an invalid handle.");
            return false;
        }

        auto& subtable = getSubTable(getSubTableType(isArray, isCube));

        if (slot >= subtable.capacity)
        {
            logWarning("Attempting to update slot ", slot, " in Vulkan texture table that is out-of-bounds (max = ", subtable.capacity, ")");
            return false;
        }

        bool updatedExistingSlot = false;
        bool isReservedSlot = slot < static_cast<uint32_t>(TextureTableReservedIndices::ReservedIndicesCount);      // The free count already takes into account the reserved indices in build()

        if (subtable.slotOwners[slot].isValid())
        {
            // Updating an existing occupied slot. Capacity is already accounted for.
            subtable.slotOwners[slot] = handle;
            updatedExistingSlot = true;
        }
        else
        {
            // Updating an unoccupied slot. This is typically avoided except for the engine reserved default textures.
            std::erase(subtable.freeSlots, slot);
            subtable.slotOwners[slot] = handle;
            
            if (!isReservedSlot)
            {
                subtable.freeCount--;
            }
        }

        if (!writeSlot(slot, m_pContext->resources.getTexture(handle)))
        {
            if (!updatedExistingSlot)
            {
                if (!isReservedSlot)
                {
                    subtable.freeCount++;
                }

                subtable.freeSlots.push_back(slot);
            }

            subtable.slotOwners[slot] = {};

            return false;
        }

        if (slot == 0u)
        {
            subtable.fallbackHandle = handle;
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

    TextureTable::TextureSubTableType TextureTable::getSubTableType(bool isArray, bool isCube) const noexcept
    {
        if (isCube)
        {
            return TextureSubTableType::Texture2DCubeArray;
        }
        else if (isArray)
        {
            return TextureSubTableType::Texture2DArray;
        }
        else
        {
            return TextureSubTableType::Texture2D;
        }
    }

    TextureTable::TextureSubTable& TextureTable::getSubTable(TextureSubTableType type) noexcept
    {
        LITL_FATAL_ASSERT(static_cast<size_t>(type) < static_cast<size_t>(TextureSubTableType::Count));
        return m_subtables[static_cast<size_t>(type)];
    }
}