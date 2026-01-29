#ifndef LITL_COMMON_H__
#define LITL_COMMON_H__

#include <cstdint>

namespace LITL::Renderer
{
    // Contexts
    struct RenderContext;
    struct ShaderContext;
    struct CommandBufferContext;
    struct MeshContext;

    // Classes
    class Shader;
    class ShaderProgram;
    enum class ShaderStage;
    class CommandBuffer;
    class Mesh;

    // Descriptors and Results
    struct GetMemoryTypeDescriptor;
    struct GetMemoryTypeResult;
}

#endif