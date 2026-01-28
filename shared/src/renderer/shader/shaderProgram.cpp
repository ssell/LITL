#include "renderer/shader/shaderProgram.hpp"
#include "renderer/common.inl"

namespace LITL::Renderer
{
    ShaderProgram::ShaderProgram(RenderContext const* const context)
        : m_vertexShader({ context }), 
          m_preTessellationShader({ context }), 
          m_postTessellationShader({ context }), 
          m_geometryShader({ context }), 
          m_fragmentShader({ context }), 
          m_computeShader({ context })
    {

    }

    bool ShaderProgram::loadVertexShader(const char* spirvPath)
    {
        return m_vertexShader.load(spirvPath, "vertexMain");
    }

    bool ShaderProgram::loadPreTessellationShader(const char* spirvPath)
    {
        return m_preTessellationShader.load(spirvPath, "preTessellationMain");
    }

    bool ShaderProgram::loadPostTessellationShader(const char* spirvPath)
    {
        return m_postTessellationShader.load(spirvPath, "postTessellationMain");
    }

    bool ShaderProgram::loadGeometryShader(const char* spirvPath)
    {
        return m_geometryShader.load(spirvPath, "geometryMain");
    }

    bool ShaderProgram::loadFragmentShader(const char* spirvPath)
    {
        return m_fragmentShader.load(spirvPath, "fragmentMain");
    }

    bool ShaderProgram::loadComputeShader(const char* spirvPath)
    {
        return m_computeShader.load(spirvPath, "computeMain");
    }

    VertexShader const* ShaderProgram::getVertexShader() const noexcept
    {
        return &m_vertexShader;
    }

    PreTessellationShader const* ShaderProgram::getPreTessellationShader() const noexcept
    {
        return &m_preTessellationShader;
    }

    PostTessellationShader const* ShaderProgram::getPostTessellationShader() const noexcept
    {
        return &m_postTessellationShader;
    }

    GeometryShader const* ShaderProgram::getGeometryShader() const noexcept
    {
        return &m_geometryShader;
    }

    FragmentShader const* ShaderProgram::getFragmentShader() const noexcept
    {
        return &m_fragmentShader;
    }

    ComputeShader const* ShaderProgram::getComputeShader() const noexcept
    {
        return &m_computeShader;
    }

}