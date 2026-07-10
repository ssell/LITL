#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/resources/utility/descriptorSetAllocator.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    void DescriptorSetAllocator::build(RendererContext& context, uint32_t setsPerPool, std::span<VkDescriptorPoolSize const> sizesPerPool) noexcept
    {
        m_pContext = &context;
        m_setsPerPool = setsPerPool;
        m_sizesPerPool.assign(sizesPerPool.begin(), sizesPerPool.end());
    }

    void DescriptorSetAllocator::destroy() noexcept
    {
        if (m_currentPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(m_pContext->device.vkDevice, m_currentPool, nullptr);
            m_currentPool = VK_NULL_HANDLE;
        }

        for (auto freePool : m_freePools)
        {
            vkDestroyDescriptorPool(m_pContext->device.vkDevice, freePool, nullptr);
        }

        for (auto usedPool : m_usedPools)
        {
            vkDestroyDescriptorPool(m_pContext->device.vkDevice, usedPool, nullptr);
        }

        m_freePools.clear();
        m_usedPools.clear();
    }

    VkDescriptorSet DescriptorSetAllocator::allocate(VkDescriptorSetLayout layout) noexcept
    {
        if (m_currentPool == VK_NULL_HANDLE)
        {
            m_currentPool = obtainPool();
        }

        VkDescriptorSetAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = m_currentPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &layout
        };

        VkDescriptorSet set = VK_NULL_HANDLE;
        VkResult result = vkAllocateDescriptorSets(m_pContext->device.vkDevice, &info, &set);

        if ((result == VK_ERROR_OUT_OF_POOL_MEMORY) || (result == VK_ERROR_FRAGMENTED_POOL))
        {
            // Retire current pool and grab a fresh one to retry.
            m_usedPools.push_back(m_currentPool);

            m_currentPool = obtainPool();
            info.descriptorPool = m_currentPool;
            result = vkAllocateDescriptorSets(m_pContext->device.vkDevice, &info, &set);

            LITL_ASSERT_MSG((result != VK_ERROR_OUT_OF_POOL_MEMORY) && (result != VK_ERROR_FRAGMENTED_POOL), "DescriptorSetAllocator failed to allocate descriptor set due to memory constraints.", VK_NULL_HANDLE);
        }
        else
        {
            LITL_ASSERT_MSG((result == VK_SUCCESS), "DescriptorSetALlocator failed to allocator descriptor set due to unhandled error.", VK_NULL_HANDLE);
        }

        return set;
    }

    void DescriptorSetAllocator::resetTransient() noexcept
    {
        if (m_currentPool != VK_NULL_HANDLE)
        {
            m_usedPools.push_back(m_currentPool);
            m_currentPool = VK_NULL_HANDLE;
        }

        for (auto pool : m_usedPools)
        {
            vkResetDescriptorPool(m_pContext->device.vkDevice, pool, 0u);
            m_freePools.push_back(pool);
        }

        m_usedPools.clear();
    }

    VkDescriptorPool DescriptorSetAllocator::obtainPool() noexcept
    {
        if (!m_freePools.empty())
        {
            VkDescriptorPool freePool = m_freePools.back();
            m_freePools.pop_back();
            return freePool;
        }
        else
        {
            return createPool();
        }
    }

    VkDescriptorPool DescriptorSetAllocator::createPool() noexcept
    {
        const VkDescriptorPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = m_setsPerPool,
            .poolSizeCount = static_cast<uint32_t>(m_sizesPerPool.size()),
            .pPoolSizes = m_sizesPerPool.data()
        };

        VkDescriptorPool pool = VK_NULL_HANDLE;
        const VkResult result = vkCreateDescriptorPool(m_pContext->device.vkDevice, &info, nullptr, &pool);

        LITL_ASSERT_MSG(result == VK_SUCCESS, "DescriptorSetAllocator failed to create new VkDescriptorPool", VK_NULL_HANDLE);

        return pool;
    }
}