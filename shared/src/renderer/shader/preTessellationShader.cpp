#include "renderer/shader/preTessellationShader.hpp"

namespace LITL::Renderer
{
    PreTessellationShader::PreTessellationShader(RenderContext const* const context)
        : Shader(context, ShaderStage::PreTessellation)
    {

    }
}