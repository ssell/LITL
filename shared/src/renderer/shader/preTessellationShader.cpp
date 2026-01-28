#include "renderer/shader/preTessellationShader.hpp"

namespace LITL
{
    PreTessellationShader::PreTessellationShader(RenderContext const* const context)
        : Shader(context, ShaderStage::PreTessellation)
    {

    }
}