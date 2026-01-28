#include "renderer/shader/postTessellationShader.hpp"

namespace LITL::Renderer
{
    PostTessellationShader::PostTessellationShader(RenderContext const* const context)
        : Shader(context, ShaderStage::PostTessellation)
    {

    }
}