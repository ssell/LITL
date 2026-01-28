#ifndef LITL_SHADER_PROGRAM_H__
#define LITL_SHADER_PROGRAM_H__

#include "renderer/shader/vertexShader.hpp"
#include "renderer/shader/preTessellationShader.hpp"
#include "renderer/shader/postTessellationShader.hpp"
#include "renderer/shader/geometryShader.hpp"
#include "renderer/shader/fragmentShader.hpp"
#include "renderer/shader/computeShader.hpp"

namespace LITL::Renderer
{
    class ShaderProgram
    {
    public:

        ShaderProgram(RenderContext const* const context);
        ShaderProgram(ShaderProgram const& other) = delete;
        ShaderProgram& operator=(ShaderProgram other) = delete;

        bool loadVertexShader(const char* spirvPath);
        bool loadPreTessellationShader(const char* spirvPath);
        bool loadPostTessellationShader(const char* spirvPath);
        bool loadGeometryShader(const char* spirvPath);
        bool loadFragmentShader(const char* spirvPath);
        bool loadComputeShader(const char* spirvPath);

        VertexShader const* getVertexShader() const noexcept;
        PreTessellationShader const* getPreTessellationShader() const noexcept;
        PostTessellationShader const* getPostTessellationShader() const noexcept;
        GeometryShader const* getGeometryShader() const noexcept;
        FragmentShader const* getFragmentShader() const noexcept;
        ComputeShader const* getComputeShader() const noexcept;

    private:

        VertexShader m_vertexShader;
        PreTessellationShader m_preTessellationShader;
        PostTessellationShader m_postTessellationShader;
        GeometryShader m_geometryShader;
        FragmentShader m_fragmentShader;
        ComputeShader m_computeShader;
    };
}

#endif 