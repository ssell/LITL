#ifndef LITL_RENDERER_VULKAN_DESCRIPTOR_SET_CHANGE_TRACKER_H__
#define LITL_RENDERER_VULKAN_DESCRIPTOR_SET_CHANGE_TRACKER_H__

#include <array>
#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"

namespace litl::vulkan
{
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

    public:

        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorBufferInfo bufferInfo) noexcept;
        void addChange(uint32_t binding, uint32_t set, VkDescriptorType type, VkDescriptorImageInfo imageInfo) noexcept;
        void flushChanges(uint32_t set) noexcept;

    private:

        uint32_t m_dirtyMask = 0u;
        std::array<std::vector<DescriptorSetChange>, 32ull> m_pendingChanges;

    };
}

#endif