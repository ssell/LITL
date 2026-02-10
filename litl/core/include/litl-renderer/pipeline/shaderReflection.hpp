#ifndef LITL_RENDERER_SHADER_REFLECTION_H__
#define LITL_RENDERER_SHADER_REFLECTION_H__

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "litl-renderer/pipeline/shaderEnums.hpp"

namespace LITL::Renderer
{
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
        /// If the resource is not an array, size is 1.
        /// </summary>
        uint32_t arraySize;

        /// <summary>
        /// For buffer validation only.
        /// </summary>
        uint32_t sizeBytes;
    };

    /// <summary>
    /// Used with push constants to send small amounts of data directly to one or more stages.
    /// </summary>
    struct PushConstantRange
    {
        uint32_t offset;
        uint32_t sizeBytes;
    };

    /// <summary>
    /// Describes a variable used as either input to a Vertex shader or output from a Fragment shader.
    /// </summary>
    struct ShaderInputOutputVariable
    {
        /// <summary>
        /// Optional semantic name.
        /// </summary>
        std::string name;

        /// <summary>
        /// Maps to `layout(location)`
        /// </summary>
        uint32_t location;

        /// <summary>
        /// The scalar type (float, uint, etc.)
        /// </summary>
        ShaderScalarType scalarType;

        /// <summary>
        /// Number of scalars in the attribute. For example float vs vec2 vs vec3 vs vec4.
        /// </summary>
        uint32_t componentCount;
    };

    /// <summary>
    /// For use with specialization constants: https://docs.vulkan.org/samples/latest/samples/performance/specialization_constants/README.html
    /// </summary>
    struct SpecializationConstant
    {
        std::string name;
        uint32_t id;
        ShaderScalarType scalarType;
    };

    /// <summary>
    /// Correlates to `layout(local_size_x=...)` is GLSL and `[numthreads]` in HLSL
    /// </summary>
    struct ComputeInfo
    {
        uint32_t localSizeX;
        uint32_t localSizeY;
        uint32_t localSizeZ;
    };

    struct ShaderReflection
    {
        ShaderStage stage;

        std::vector<ResourceBinding> resources;
        std::vector<PushConstantRange> pushConstants;
        std::vector<ShaderInputOutputVariable> vertexInputs;
        std::vector<ShaderInputOutputVariable> fragmentOutputs;
        std::vector<SpecializationConstant> specializationConstants;

        std::optional<ComputeInfo> computeInfo;
    };

    std::optional<ShaderReflection> reflectSPIRV(std::span<uint8_t const> spirvByteCode);
}

#endif