#ifndef LITL_RENDERER_SHADER_ENUMS_H__
#define LITL_RENDERER_SHADER_ENUMS_H__

#include <cstdint>

namespace LITL::Renderer
{
    /// <summary>
    /// Stage of the compiled shader program.
    /// The bit fields are also used in visibility masks.
    /// </summary>
    enum class ShaderStage : uint32_t
    {
        Unknown                = 0b00000000,
        Vertex                 = 0b00000001,
        Fragment               = 0b00000010,
        Geometry               = 0b00000100,
        TessellationControl    = 0b00001000,
        TessellationEvaluation = 0b00010000,
        Compute                = 0b00100000,
        Mesh                   = 0b01000000,
        Task                   = 0b10000000
    };

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
}

#endif