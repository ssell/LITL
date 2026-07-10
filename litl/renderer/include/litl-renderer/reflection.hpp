#ifndef LITL_RENDERER_REFLECTION_H__
#define LITL_RENDERER_REFLECTION_H__

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "litl-core/stringId.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl
{
    /// <summary>
    /// Describes a single resource bound to one or more shader stages.
    /// In Vulkan these are descriptor bindings.
    /// </summary>
    struct ResourceBinding
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
        /// For buffer validation only.
        /// </summary>
        uint32_t sizeBytes;

        /// <summary>
        /// "Camera", "AlbedoTexture", etc.
        /// </summary>
        std::string name;
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

        /// <summary>
        /// Optional semantic name.
        /// </summary>
        std::string name;
    };

    /// <summary>
    /// For use with specialization constants: https://docs.vulkan.org/samples/latest/samples/performance/specialization_constants/README.html
    /// </summary>
    struct SpecializationConstant
    {
        uint32_t id;
        ShaderScalarType scalarType;
        std::string name;
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

    /// <summary>
    /// Reflected data for a single entry point in a SPIR-V bytecode blob.
    /// 
    /// A blob may have one or more entry points, and entry points may even
    /// share the same shader stage. For example, a blob might define one
    /// vertex shader but two fragment shaders.
    /// </summary>
    struct EntryPointReflection
    {
        std::string entryPoint;
        ShaderStage stage;

        std::vector<ResourceBinding> resources;
        std::vector<PushConstantRange> pushConstants;
        std::vector<ShaderInputOutputVariable> vertexInputs;
        std::vector<ShaderInputOutputVariable> fragmentOutputs;
        std::optional<ComputeInfo> computeInfo;
    };

    /// <summary>
    /// Collection of reflected data about a shader.
    /// </summary>
    struct ShaderReflection
    {
        /**
         * Note on the entryPoints vector below.
         *
         * We could change it to a flat_map (well, our flatHashMap since flat_map is C++23) with a key
         * on the entry point name or add a separate name-based map indexing the index. Which is what Claude suggests:
         *
         *     "The entry-point lookup ergonomics — reflection's entryPoints is a vector you'll search by name on every pipeline creation.
         *      If you anticipate lots of pipelines, swap it for flat_map-style sorted vector or a small open-address map."
         * 
         * However, rudimentary performance tests show the linear scan of the vector to match the flat hash map when the
         * number of entries is low (~10) and to beat it when the count is really low (~5). Considering an average shader blob
         * is likely to only have 1-3 entry points, then the vector with the rudimentary string comparison actually wins out.
         * 
         * See "String Key Map Performance Test" in flatHashMap_tests.cpp
         */

        std::vector<EntryPointReflection> entryPoints;
        std::vector<SpecializationConstant> specializationConstants;

        /// <summary>
        /// Returns if there is at least one entry point for the specified stage.
        /// </summary>
        /// <param name="stage"></param>
        /// <returns></returns>
        [[nodiscard]] bool hasShaderStage(ShaderStage stage) const noexcept
        {
            for (auto const& entryPoint : entryPoints)
            {
                if (entryPoint.stage == stage)
                {
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Returns the first entry point for the specified stage, if one exists.
        /// </summary>
        /// <param name="stage"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<EntryPointReflection const*> getEntryPoint(ShaderStage stage) const noexcept
        {
            for (auto const& entryPoint : entryPoints)
            {
                if (entryPoint.stage == stage)
                {
                    return &entryPoint;
                }
            }

            return std::nullopt;
        }

        /// <summary>
        /// Returns the entry point that matches the specified name, if it exists.
        /// </summary>
        /// <param name="entryPointName"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<EntryPointReflection const*> getEntryPoint(std::string_view entryPointName) const noexcept
        {
            for (auto const& entryPoint : entryPoints)
            {
                if (entryPointName == entryPoint.entryPoint)
                {
                    return &entryPoint;
                }
            }

            return std::nullopt;
        }
    };

    // -------------------------------------------------------------------------------------
    // Reflection Merge Structures
    // -------------------------------------------------------------------------------------

    struct MergedResourceBinding
    {
        /// <summary>
        /// Hash of the name.
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

        /// <summary>
        /// 0 = runtime bindless array, 1 = not array, >=2 = array of declared size
        /// </summary>
        uint32_t arraySize;

        /// <summary>
        /// For buffer validation only.
        /// </summary>
        uint32_t sizeBytes;

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
        ErrorInvalidShaderStage,    // The descriptor has an invalid shader stage defined
        ErrorInvalidEntryPoint,     // The specified entry point was not found
        ErrorDuplicateStage,        // The same stage was defined more than once in the descriptor
        ErrorStageMismatch,         // The reflected stage does not match the stage specified in the descriptor
        ErrorTypeMismatch,          // Same (set, binding), different ShaderResourceType
        ErrorArraySizeMismatch,     // Same (set, binding), different arraySize
        ErrorBufferSizeMismatch,    // Same (set, binding), different buffer size
        ErrorPushConstantOverlap,   // Push constants overlap incompatibly across stages
        ErrorNoVertexStage          // Graphics pipeline with no vertex stage
    };

    // -------------------------------------------------------------------------------------
    // Reflection Operations
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Given a blob of byte-code and a shader entry point, returns the reflected shader data.
    /// </summary>
    std::optional<ShaderReflection> reflectSPIRV(std::span<std::byte const> spirvByteCode);
}

#endif