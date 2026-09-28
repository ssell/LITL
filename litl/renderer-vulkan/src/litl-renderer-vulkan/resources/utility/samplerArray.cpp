#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/resources/sampler.hpp"
#include "litl-renderer-vulkan/resources/utility/samplerArray.hpp"
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    bool SamplerArray::build(RendererContext& context) noexcept
    {
        LITL_ASSERT_MSG(m_samplerHandles.size() == SamplerPredefinedDescriptors.size(), "Mismatch between the number of expected predefined sampler descriptors.", false);

        for (uint32_t i = 0u; i < m_samplerHandles.size(); ++i)
        {
            m_samplerHandles[i] = context.resources.createSampler(SamplerPredefinedDescriptors[i]);

            auto* samplerResource = context.resources.getSampler(m_samplerHandles[i]);

            if ((samplerResource != nullptr) || (samplerResource->vkSampler == VK_NULL_HANDLE))
            {
                m_vkSamplers[i] = samplerResource->vkSampler;
            }
            else
            {
                logError("Failed to create predefined Vulkan sampler at index ", i);
                return false;
            }
        }

        return true;
    }

    void SamplerArray::destroy() noexcept
    {
        // ... intentionally empty. placeholder for if needed in the future ...
    }

    std::span<SamplerHandle const> SamplerArray::getSamplerHandles() const noexcept
    {
        return m_samplerHandles;
    }

    std::span<VkSampler const> SamplerArray::getVkSamplers() const noexcept
    {
        return m_vkSamplers;
    }
}