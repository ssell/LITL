#ifndef LITL_SHADER_H__
#define LITL_SHADER_H__

#include "renderer/common.hpp"

namespace LITL::Renderer
{
    enum class ShaderStage
    {
        Vertex = 0,
        PreTessellation = 1,
        PostTessellation = 2,
        Geometry = 3,
        Fragment = 4,
        Compute = 5
    };

    class Shader
    {
    public:

        Shader(RenderContext const* const pContext, ShaderStage shaderStage);
        ~Shader();

        Shader(Shader const& other) = delete;
        Shader& operator=(Shader other) = delete;

        bool load(const char* path, const char* entry);
        ShaderContext const* getContext() const noexcept;

    protected:

        ShaderStage m_ShaderStage;

    private:

        ShaderContext* m_pContext;
    };
}

#endif 