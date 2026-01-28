#include "renderer/shader/fragmentShader.hpp"

namespace LITL::Renderer
{
    FragmentShader::FragmentShader(RenderContext const* const context)
        : Shader(context, ShaderStage::Fragment)
    {

    }
}