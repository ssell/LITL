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
        Task                   = 0b1000'0000
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
        Unknown = 0,
        Float = 1,
        Int = 2,
        Uint = 3,
        Bool = 4
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