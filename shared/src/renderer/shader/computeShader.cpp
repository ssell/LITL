#include "renderer/shader/computeShader.hpp"

namespace LITL
{
    ComputeShader::ComputeShader(RenderContext const* const context)
        : Shader(context, ShaderStage::Compute)
    {

    }
}