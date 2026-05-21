#ifndef LITL_RENDERER_SHADER_MODULE_H__
#define LITL_RENDERER_SHADER_MODULE_H__

#include <cstdint>
#include <span>
#include <string>

#include "litl-core/handles.hpp"

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

    constexpr ShaderStage operator|(ShaderStage a, ShaderStage b) noexcept
    {
        return static_cast<ShaderStage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    constexpr ShaderStage operator&(ShaderStage a, ShaderStage b) noexcept
    {
        return static_cast<ShaderStage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }

    constexpr bool any(ShaderStage s) noexcept
    {
        return static_cast<uint32_t>(s) != 0;
    }

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
        /// Shader stage (vertex, fragment, etc.)
        /// </summary>
        ShaderStage stage;

        /// <summary>
        /// Shader function name used as the entry point.
        /// </summary>
        std::string entryPoint;

        /// <summary>
        /// Non-owning view of the compiled bytecode (SPIR-V, DXIL, etc.)
        /// </summary>
        std::span<uint8_t const> bytes;

        /// <summary>
        /// Calculated once at load time.
        /// </summary>
        uint64_t hashedEntryPoint;

        /// <summary>
        /// Calculated once at load time.
        /// </summary>
        uint64_t hashedBytes;
    };

    struct ShaderModuleTag {};
    using ShaderModuleHandle = Handle<ShaderModuleTag>;
}

#endif