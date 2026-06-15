#ifndef LITL_RENDERER_PIPELINE_RESOURCE_KEY_H__
#define LITL_RENDERER_PIPELINE_RESOURCE_KEY_H__

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "litl-renderer/resources/shaderModule.hpp"

namespace litl
{
    using PipelineResourceKey = uint64_t;

    /// <summary>
    /// Describes a single resource bound to one or more shader stages.
    /// In Vulkan these are descriptor bindings.
    /// </summary>
    struct ResourceBinding
    {
        /// <summary>
        /// "Camera", "AlbedoTexture", etc.
        /// </summary>
        std::string name;

        /// <summary>
        /// Hash of the name.
        /// </summary>
        PipelineResourceKey key = 0ull;

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

        /// <summary>
        /// 0 = runtime bindless array, 1 = not array, >=2 = array of declared size
        /// </summary>
        uint32_t arraySize;

        /// <summary>
        /// For buffer validation only.
        /// </summary>
        uint32_t sizeBytes;
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
        std::vector<ResourceBinding> resources;

        [[nodiscard]] ResourceBinding const* getResourceBinding(PipelineResourceKey key) const noexcept;
        [[nodiscard]] bool contains(PipelineResourceKey key) const noexcept;
        [[nodiscard]] static PipelineResourceKey getKey(std::string_view name) noexcept;
    };
}

#endif