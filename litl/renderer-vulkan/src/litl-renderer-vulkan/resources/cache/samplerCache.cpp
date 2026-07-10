#include <optional>

#include "litl-renderer-vulkan/resources/cache/samplerCache.hpp"
#include "litl-core/hash.hpp"

namespace litl::vulkan
{
    void SamplerCache::build(VkDevice vkDevice) noexcept
    {
        m_vkDevice = vkDevice;
    }

    SamplerCacheKey SamplerCache::hashSampler(SamplerDescriptor const& descriptor) const noexcept
    {
        SamplerCacheKey hash = 0ull;

        hashCombine64(hash, static_cast<uint64_t>(descriptor.minFilter));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.magFilter));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.mipFilter));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.addressU));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.addressV));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.addressW));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.anisotropy));

        if (descriptor.compareOp.has_value())
        {
            hashCombine64(hash, static_cast<uint64_t>(descriptor.compareOp.value()));
        }

        hashCombine64(hash, static_cast<uint64_t>(descriptor.border));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.lodBias));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.minLod));
        hashCombine64(hash, static_cast<uint64_t>(descriptor.maxLod));

        return hash;
    }

    SamplerHandle SamplerCache::getSampler(SamplerCacheKey key) const noexcept
    {
        for (auto& cached : m_samplers)
        {
            if (cached.key == key)
            {
                return cached.handle;
            }
        }

        return {};
    }

    void SamplerCache::setSampler(SamplerCacheKey key, SamplerHandle sampler) noexcept
    {
        m_samplers.emplace_back(sampler, key);
    }

    void SamplerCache::removeSampler(SamplerHandle sampler) noexcept
    {
        std::optional<size_t> index = std::nullopt;

        for (size_t i = 0ull; i < m_samplers.size(); ++i)
        {
            if (m_samplers[i].handle == sampler)
            {
                index = i;
                break;
            }
        }

        if (index != std::nullopt)
        {
            m_samplers[index.value()] = m_samplers.back();
            m_samplers.pop_back();
        }
    }
}