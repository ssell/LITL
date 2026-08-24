#ifndef LITL_RENDERER_SHADER_MODULE_H__
#define LITL_RENDERER_SHADER_MODULE_H__

#include <cstdint>
#include <span>
#include <string>

#include "litl-core/handles.hpp"
#include "litl-core/enumBitFlags.hpp"

namespace litl
{
    /// <summary>
    /// Stage of the compiled shader program.
    /// The bit fields are also used in visibility masks.
    /// </summary>
    enum class ShaderStage : uint32_t
    {
        None                   = 0b0000'0000,
        Vertex                 = 0b0000'0001,
        Fragment               = 0b0000'0010,
        Geometry               = 0b0000'0100,
        TessellationControl    = 0b0000'1000,
        TessellationEvaluation = 0b0001'0000,
        Compute                = 0b0010'0000,
        Mesh                   = 0b0100'0000,
        Task                   = 0b1000'0000,
        All                    = ~0u
    };

    LITL_ENABLE_BITMASK(ShaderStage);

    /// <summary>
    /// Resource types that can be bound to shader stages.
    /// </summary>
    enum class ShaderResourceType : uint32_t
    {
        Unknown = 0,
        Sampler = 1,
        UniformBuffer = 2,
        StorageBuffer = 3,
        ImageBuffer = 4,
        SampledImage = 5,
        StorageImage = 6,
        InputAttachment = 7,
        AccelerationStructure = 8
    };

    /// <summary>
    /// Scalar types that can be bound to shader stages.
    /// </summary>
    enum class ShaderScalarType : uint32_t
    {
        Unknown = 0u,
        Bool = 1u,
        Integer = 2u,
        Float = 3u
    };

    enum class ShaderVariableFlagBits : uint32_t
    {
        Undefined    = 0u,
        Void         = 1 << 0,
        Bool         = 1 << 1,
        Int          = 1 << 2,
        Float        = 1 << 3,
        Vector       = 1 << 4,
        Matrix       = 1 << 5,
        Image        = 1 << 6,
        Sampler      = 1 << 7,
        SampledImage = 1 << 8,
        Block        = 1 << 9,
        Acceleration = 1 << 10,
        Struct       = 1 << 11,
        Array        = 1 << 12,
        Ref          = 1 << 13,
        Unsigned     = 1 << 14
    };

    LITL_ENABLE_BITMASK(ShaderVariableFlagBits);
    using ShaderVariableFlag = ShaderVariableFlagBits;

    struct ShaderVariable
    {
        static constexpr uint32_t MaxArrayDimensions = 4u;

        /// <summary>
        /// The base scalar type of the variable.
        /// For both `float` and `float3` this would be `Float`.
        /// </summary>
        ShaderScalarType scalarType = ShaderScalarType::Unknown;

        /// <summary>
        /// Flags defining the variable.
        /// For example:
        /// 
        ///     Float                  = float
        ///     Float | Vector         = floatN (float2, float3, float4, etc.)
        ///     Float | Array          = float[]
        ///     Float | Vector | Array = floatN[] (float2[], float3[], float4[])
        /// </summary>
        ShaderVariableFlag flag = ShaderVariableFlagBits::Undefined;

        /// <summary>
        /// The byte size of an individual component of the variable type.
        /// For both `float` and `float3` this would be 4.
        /// </summary>
        uint32_t size = 0u;

        /// <summary>
        /// The number of individual components that make up the variable type.
        /// For `float` this would be 1, for `float3` this would be 3.
        /// </summary>
        uint32_t componentCount = 0u;

        /// <summary>
        /// The physical byte distance between the start of one matrix column and the next.
        /// </summary>
        uint32_t matrixStride = 0u;

        /// <summary>
        /// The physical byte distance between the start of one array element and the start of the next element.
        /// </summary>
        uint32_t arrayStride = 0u;

        /// <summary>
        /// The number of array dimensions.
        /// </summary>
        uint32_t arrayDimensionsCount = 0u;

        /// <summary>
        /// The logical element count (length) of each dimension in an array.
        /// A value of 0 in a valid index indicates a runtime-sized array.
        /// </summary>
        uint32_t arrayDimensions[MaxArrayDimensions] = {};
    };

    /// <summary>
    /// What each set within a DescriptorSetLayout correlates to.
    /// </summary>
    enum class DescriptorSetIndex : uint32_t
    {
        /// <summary>
        /// Descriptor set that is consistent the entire frame.
        /// Example: (view matrix, projection matrix, time, camera position, frame uniforms)
        /// </summary>
        PerFrame = 0,

        /// <summary>
        /// Descriptor set that is consistent for an entire render pass.
        /// Example: opaque = (shadow maps, environment data) or transparent = (depth buffer)
        /// </summary>
        PerPass = 1,

        /// <summary>
        /// Descriptor set shared by all objects using the same material.
        /// Example: skybox = (cloud texture) or opaque = (material textures) or transparent = (smoke texture, noise texture)
        /// </summary>
        PerMaterial = 2,

        /// <summary>
        /// Descriptor set specific to each object though the layout may be implicitly shared by other objects.
        /// Example: (object index, instance data)
        /// </summary>
        PerObject = 3,

        DescriptorSetMaxCount = 4
    };

    /// <summary>
    /// Generic description of a shader module.
    /// </summary>
    struct ShaderModuleDescriptor
    {
        /// <summary>
        /// For now, the file path. In the future the asset name once the asset system is in place.
        /// </summary>
        std::string resource;

        /// <summary>
        /// Non-owning view of the compiled bytecode (SPIR-V, DXIL, etc.)
        /// The data must be valid until shader module creation is complete.
        /// Once the shader module is constructed the bytecode itself is 
        /// no longer referenced/used, only it's hash.
        /// </summary>
        std::span<std::byte const> bytes;
    };

    struct ShaderModuleTag {};
    using ShaderModuleHandle = Handle<ShaderModuleTag>;

    /// <summary>
    /// The shader module and entry point into that module.
    /// The specified stage must match the reflected stage of the entry point.
    /// </summary>
    struct PipelineShaderDescriptor
    {
        ShaderModuleHandle handle{};
        ShaderStage stage;
        std::string entryPoint;
    };
}

#endif