#ifndef LITL_RENDERER_VULKAN_DESCRIPTOR_SET_ALLOCATOR_H__
#define LITL_RENDERER_VULKAN_DESCRIPTOR_SET_ALLOCATOR_H__

#include <span>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    class DescriptorSetAllocator
    {
    public:

        DescriptorSetAllocator() = default;
        ~DescriptorSetAllocator() = default;

        DescriptorSetAllocator(DescriptorSetAllocator const&) = delete;
        DescriptorSetAllocator& operator=(DescriptorSetAllocator const&) = delete;

        void build(RendererContext& context, uint32_t setsPerPool, std::span<VkDescriptorPoolSize const> sizesPerPool) noexcept;
        void destroy() noexcept;

        [[nodiscard]] VkDescriptorSet allocate(VkDescriptorSetLayout layout) noexcept;
        void resetTransient() noexcept;

    private:

        VkDescriptorPool obtainPool() noexcept;
        VkDescriptorPool createPool() noexcept;

        RendererContext* m_pContext = nullptr;

        uint32_t m_setsPerPool = 0u;
        std::vector<VkDescriptorPoolSize> m_sizesPerPool;

        VkDescriptorPool m_currentPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorPool> m_usedPools;
        std::vector<VkDescriptorPool> m_freePools;
    };
}

#endif