#ifndef LITL_COMPUTE_SHADER_H__
#define LITL_COMPUTE_SHADER_H__

#include "renderer/shader/shader.hpp"

namespace LITL
{
    class ComputeShader : public Shader
    {
    public:

        ComputeShader(RenderContext const* const context);

    protected:

    private:
    };
}

#endif 