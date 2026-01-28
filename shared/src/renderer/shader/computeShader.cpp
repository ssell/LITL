#include "renderer/shader/computeShader.hpp"

namespace LITL::Renderer
{
    ComputeShader::ComputeShader(RenderContext const* const context)
        : Shader(context, ShaderStage::Compute)
    {

    }
}