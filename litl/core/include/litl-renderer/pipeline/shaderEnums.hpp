#ifndef LITL_RENDERER_SHADER_ENUMS_H__
#define LITL_RENDERER_SHADER_ENUMS_H__

#include <cstdint>

namespace LITL::Renderer
{
    enum class ShaderStage : uint32_t
    {
        Vertex                 = 0b0001,
        Fragment               = 0b0010,
        Geometry               = 0b0100,
        TessellationControl    = 0b00001000,
        TessellationEvaluation = 0b00010000,
        Compute                = 0b00100000,
        Mesh                   = 0b01000000,
        Task                   = 0b10000000
    };

    enum class ShaderResourceType : uint32_t
    {
        Unknown = 0,
        UniformBuffer = 1,
        StorageBuffer = 2,
        SampledImage = 3,
        StorageImage = 4,
        Sampler = 5,
        AccelerationStructure = 6
    };

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