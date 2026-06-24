#ifndef LITL_RENDERER_VULKAN_PIPELINE_LAYOUT_CACHE_H__
#define LITL_RENDERER_VULKAN_PIPELINE_LAYOUT_CACHE_H__

#include <unordered_map>
#include <vector>

#include "litl-core/hash.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/resources/pipelineLayoutDescriptor.hpp"

namespace litl::vulkan
{
    struct PipelineLayoutCacheKey
    {
        std::vector<VkDescriptorSetLayout> setLayoutHandles;
        std::vector<MergedPushConstantRange> pushConstants;

        bool operator==(PipelineLayoutCacheKey const&) const = default;
    };
}

// Provide custom hashing to use with std::unordered_map
namespace std
{
    template<>
    struct hash<litl::vulkan::PipelineLayoutCacheKey>
    {
        std::size_t operator()(litl::vulkan::PipelineLayoutCacheKey const& layout) const noexcept
        {
            std::size_t h = 0ull;

            static_assert(sizeof(VkDescriptorSetLayout) == sizeof(uint64_t), "litl::vulkan::PipelineLayoutCacheKey expects VkDescriptorSetLayout to be 64 bits");

            litl::hashCombine64(h, layout.setLayoutHandles.size());
            for (auto const& layoutHandle : layout.setLayoutHandles) { litl::hashCombine64(h, reinterpret_cast<uint64_t>(layoutHandle)); }

            litl::hashCombine64(h, layout.pushConstants.size());
            for (auto const& pushConstant : layout.pushConstants) { litl::hashCombine64(h, litl::hashPOD(pushConstant)); }

            return h;
        }
    };
}

namespace litl::vulkan
{
    /// <summary>
    /// A single-device cache.
    /// 
    /// The device is captured at build() and used for all subsequent creation/destruction. 
    /// Calling initialize() twice (without destroy() in between) is not supported.
    /// 
    /// TODO: 
    ///     thread-safety: currently not needed as all work is done at a sync point, but task pools may be added in the future for shader compilations, etc.
    ///     compatability introspection: can avoid redundant vkCmdBindDescriptorSets calls if we can check if the requested set is compatible with the one already bound.
    /// </summary>
    class PipelineLayoutCache
    {
    public:

        PipelineLayoutCache() = default;
        ~PipelineLayoutCache() = default;

        PipelineLayoutCache(PipelineLayoutCache const&) = delete;
        PipelineLayoutCache& operator=(PipelineLayoutCache const&) = delete;

        void build(VkDevice vkDevice) noexcept;
        void destroy() noexcept;

        [[nodiscard]] VkDescriptorSetLayout getOrCreateSetLayout(DescriptorSetLayoutDesc const& descriptorSetLayoutDesc, DescriptorSetIndex index) noexcept;
        [[nodiscard]] VkPipelineLayout getOrCreatePipelineLayout(PipelineLayoutDescriptor const& pipelineLayoutDesc) noexcept;

    private:

        VkDevice m_vkDevice{ VK_NULL_HANDLE };

        std::unordered_map<DescriptorSetLayoutDesc, VkDescriptorSetLayout> m_descriptorSetLayoutMap;
        std::unordered_map<PipelineLayoutCacheKey, VkPipelineLayout> m_pipelineLayoutMap;
    };
}

#endif