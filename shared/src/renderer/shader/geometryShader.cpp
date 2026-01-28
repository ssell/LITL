#include "renderer/shader/geometryShader.hpp"

namespace LITL::Renderer
{
    GeometryShader::GeometryShader(RenderContext const* const context)
        : Shader(context, ShaderStage::Geometry)
    {

    }
}