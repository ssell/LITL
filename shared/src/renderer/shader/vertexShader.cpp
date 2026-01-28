#include "renderer/shader/vertexShader.hpp"

namespace LITL::Renderer
{
    VertexShader::VertexShader(RenderContext const* const context)
        : Shader(context, ShaderStage::Vertex)
    {

    }
}