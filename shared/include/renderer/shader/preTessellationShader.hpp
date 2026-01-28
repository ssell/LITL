#ifndef LITL_PRE_TESSELLATION_SHADER_H__
#define LITL_PRE_TESSELLATION_SHADER_H__

#include "renderer/shader/shader.hpp"

namespace LITL::Renderer
{
    class PreTessellationShader : public Shader
    {
    public:

        PreTessellationShader(RenderContext const* const context);

    protected:

    private:
    };
}

#endif 