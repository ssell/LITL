#ifndef LITL_RENDERER_VULKAN_SAMPLER_ARRAY_H__
#define LITL_RENDERER_VULKAN_SAMPLER_ARRAY_H__

#include <array>
#include <cstdint>
#include <span>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/sampler.hpp"

namespace litl::vulkan
{
    struct RendererContext;

    /// <summary>
    /// A fixed array of predefined texture samplers.
    /// This is bound at binding 0 (PerFrame) and set 1 for our standard Vulkan 1.4+ rendering path.
    /// </summary>
    class SamplerArray final
    {
    public:

        [[nodiscard]] bool build(RendererContext& context) noexcept;
        void destroy() noexcept;

        [[nodiscard]] std::span<SamplerHandle const> getSamplerHandles() const noexcept;

    private:

        std::array<SamplerHandle, static_cast<uint32_t>(SamplerPredefines::PredefinedSamplerCount)> m_samplerHandles;
    };
}

#endif