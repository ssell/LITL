#ifndef LITL_RENDERER_SHADER_REFLECTION_MERGE_H__
#define LITL_RENDERER_SHADER_REFLECTION_MERGE_H__

#include <cstdint>
#include "litl-renderer/pipeline/shader/reflection.hpp"

namespace litl
{
    struct MergedResourceBinding
    {
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
        /// Bitmask of one or more ShaderStage that reference this binding.
        /// </summary>
        ShaderStage stages;
    };

    struct MergedPushConstantRange
    {
        /// <summary>
        /// The offset applied to the constant.
        /// </summary>
        uint32_t offset;

        /// <summary>
        /// Size of the constant in bytes.
        /// </summary>
        uint32_t sizeBytes;

        /// <summary>
        /// Bitmask of one or more ShaderStage that reference this constant.
        /// </summary>
        ShaderStage stages;

        bool operator==(MergedPushConstantRange const&) const = default;
    };

    enum class MergeShaderReflectionResult : uint8_t
    {
        Success = 0,
        ErrorTypeMismatch,        // same (set, binding), different ShaderResourceType
        ErrorArraySizeMismatch,   // same (set, binding), incompatible arraySize
        ErrorPushConstantOverlap, // push constants overlap incompatibly across stages
        ErrorDuplicateStage,      // same stage reflected twice
        ErrorNoVertexStage        // graphics pipeline with no vertex stage
    };
}

#endif