#include <array>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/resources/utility/textureTable.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    bool TextureTable::build(RendererContext& context) noexcept
    {
        m_vkDevice = context.device.vkDevice;
        m_capacity = context.device.textureTableCapacity;
        m_slotOwners.resize(m_capacity, {});

        if (!buildDescriptorPool() ||
            !buildDescriptorSetLayout() ||
            !buildDescriptorSet())
        {
            return false;
        }

        return true;
    }

    bool TextureTable::buildDescriptorPool() noexcept
    {
        // TODO add a VK_DESCRIPTOR_TYPE_SAMPLER entry sized to the fixed sample-array count of 8 or 16 when samplers are added.
        const std::array<VkDescriptorPoolSize, 1> descriptorPoolSizes = {
            VkDescriptorPoolSize{
                .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = m_capacity
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
        const VkResult result = vkCreateDescriptorPool(m_vkDevice, &createDescriptorPoolInfo, nullptr, &m_vkDescriptorPool);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorPool for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    bool TextureTable::buildDescriptorSetLayout() noexcept
    {
        const std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
            VkDescriptorSetLayoutBinding {
                .binding = 0u,
                .descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = m_capacity,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr
            }
        };

        const std::array< VkDescriptorBindingFlags, 1> bindingFlags = {
            VkDescriptorBindingFlags { VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT }
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
        const VkResult result = vkCreateDescriptorSetLayout(m_vkDevice, &createInfo, nullptr, &m_vkDescriptorSetLayout);

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
        const VkResult result = vkAllocateDescriptorSets(m_vkDevice, &createInfo, &m_vkDescriptorSet);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create VkDescriptorSet for Vulkan TextureTable with result ", result);
            return false;
        }

        return true;
    }

    void TextureTable::destroy() noexcept
    {
        if (m_vkDevice != VK_NULL_HANDLE)
        {
            if (m_vkDescriptorSetLayout != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorSetLayout(m_vkDevice, m_vkDescriptorSetLayout, nullptr);
                m_vkDescriptorSetLayout = VK_NULL_HANDLE;
            }

            if (m_vkDescriptorPool != VK_NULL_HANDLE)
            {
                vkDestroyDescriptorPool(m_vkDevice, m_vkDescriptorPool, nullptr);
                m_vkDescriptorPool = VK_NULL_HANDLE;
            }

            m_vkDevice = VK_NULL_HANDLE;
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

        if (m_capacity == 0u)
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

        m_capacity--;
        m_slotOwners[slot] = handle;

        return slot;
    }

    bool TextureTable::release(uint32_t slot) noexcept
    {
        if (slot >= m_slotOwners.size())
        {
            logWarning("Attempting to free slot ", slot, " in Vulkan TextureTable that is out-of-bounds (max = ", m_slotOwners.size(), ")");
            return false;
        }

        if (!m_slotOwners[slot].isValid())
        {
            logWarning("Attempting to free slot in Vulkan TextureTable that is not currently owned.");
            return false;
        }

        m_slotOwners[slot] = {};
        m_freeSlots.push_back(slot);
        m_capacity++;

        return true;
    }

    bool TextureTable::update(uint32_t slot, TextureResourceHandle handle) noexcept
    {
        if (slot >= m_slotOwners.size())
        {
            logWarning("Attempting to update slot ", slot, " in Vulkan TextureTable that is out-of-bounds (max = ", m_slotOwners.size(), ")");
            return false;
        }

        m_slotOwners[slot] = handle;

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
}