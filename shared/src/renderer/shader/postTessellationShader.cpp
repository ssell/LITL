#include "renderer/shader/postTessellationShader.hpp"

namespace LITL
{
    PostTessellationShader::PostTessellationShader(RenderContext const* const context)
        : Shader(context, ShaderStage::PostTessellation)
    {

    }
}