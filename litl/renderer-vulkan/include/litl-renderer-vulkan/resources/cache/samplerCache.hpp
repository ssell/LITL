#ifndef LITL_RENDERER_VULKAN_SAMPLER_CACHE_H__
#define LITL_RENDERER_VULKAN_SAMPLER_CACHE_H__

#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/resources/sampler.hpp"

namespace litl::vulkan
{
    using SamplerCacheKey = uint64_t;

    struct CachedSampler
    {
        SamplerHandle handle{};
        SamplerCacheKey key;
    };
    
    class SamplerCache
    {
    public:

        SamplerCache() = default;
        ~SamplerCache() = default;

        SamplerCache(SamplerCache const&) = delete;
        SamplerCache& operator=(SamplerCache const&) = delete;

        void build(VkDevice vkDevice) noexcept;
        [[nodiscard]] SamplerCacheKey hashSampler(SamplerDescriptor const& descriptor) const noexcept;
        [[nodiscard]] SamplerHandle getSampler(SamplerCacheKey key) const noexcept;
        void setSampler(SamplerCacheKey key, SamplerHandle sampler) noexcept;
        void removeSampler(SamplerHandle sampler) noexcept;

    private:

        VkDevice m_vkDevice{ VK_NULL_HANDLE };

        /// <summary>
        /// Samplers are stored in a simple vector as the expected number of samplers is in the 10s,
        /// at which point the linear scan of the vector for the hash performs at least equal to a flat hash map.
        /// </summary>
        std::vector<CachedSampler> m_samplers;
    };
}

#endif