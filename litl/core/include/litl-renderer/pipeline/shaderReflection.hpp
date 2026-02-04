#ifndef LITL_RENDERER_SHADER_REFLECTION_H__
#define LITL_RENDERER_SHADER_REFLECTION_H__

#include <vector>
#include "litl-renderer/pipeline/shaderStage.hpp"

namespace LITL::Renderer
{
    struct DescriptorBinding
    {
        // ... todo ...
    };

    struct VertexAttribute
    {
        /// ... todo ...
    };

    struct ShaderReflection
    {
        ShaderStage stage;
        std::vector<DescriptorBinding> descriptorBindings;
        std::vector<VertexAttribute> vertexAttributes;
    };
}

#endif