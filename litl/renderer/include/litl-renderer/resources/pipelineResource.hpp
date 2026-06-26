#ifndef LITL_RENDERER_PIPELINE_RESOURCE_KEY_H__
#define LITL_RENDERER_PIPELINE_RESOURCE_KEY_H__

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "litl-core/stringId.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl
{
    struct PipelineResourceBinding
    {
        /// <summary>
        /// Hash of the string name.
        /// </summary>
        StringId id;

        /// <summary>
        /// Buffer, image, sampler, etc.
        /// </summary>
        ShaderResourceType type;

        /// <summary>
        /// Vulkan set, D3D12 space.
        /// </summary>
        uint32_t set;

        /// <summary>
        /// Vulkan binding, D3D12 register.
        /// </summary>
        uint32_t binding;

        [[nodiscard]] constexpr bool isPushCompatible() const noexcept
        {
            // Pushes can only be done to a single set, and for LITL that is the PerObject (index 3) set as it has the most potential churn.
            return static_cast<DescriptorSetIndex>(set) == DescriptorSetIndex::PerObject;
        }
    };

    /// <summary>
    /// A map of resources bound in a pipeline.
    /// 
    /// Internally uses a vector (instead of some formal map) as the expected average bound
    /// resource count is suitably low enough such that the linear scan of a vector typically
    /// outperforms the hashing/lookup operations of even a flat hash map.
    /// </summary>
    struct PipelineResourceMap
    {
        std::vector<PipelineResourceBinding> resources;

        [[nodiscard]] PipelineResourceBinding const* getResourceBinding(StringId key) const noexcept;
        [[nodiscard]] bool contains(StringId key) const noexcept;
    };
}

#endif