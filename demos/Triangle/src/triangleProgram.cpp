#include "triangleProgram.hpp"

namespace LITL
{
    // NEXT IS https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/04_Swap_chain_recreation.html

    TriangleProgram::TriangleProgram()
        : m_pShaderProgram(nullptr), m_pCommandBuffer(nullptr)
    {

    }

    bool TriangleProgram::initialize()
    {
        if (!m_renderer.initialize(1024, 768, this, "Triangle Demo"))
        {
            return false;
        }

        m_pShaderProgram = m_renderer.createShaderProgram();
        
        if (!m_pShaderProgram->loadVertexShader("assets/shaders/spirv/flat.spv"))
        {
            return false;
        }

        if (!m_pShaderProgram->loadFragmentShader("assets/shaders/spirv/flat.spv"))
        {
            return false;
        }

        if (!m_renderer.recreateShaderPipeline(&m_pShaderProgram, 1))
        {
            return false;
        }

        m_pCommandBuffer = m_renderer.createCommandBuffer();

        return true;
    }

    void TriangleProgram::shutdown()
    {
        if (m_pShaderProgram != nullptr)
        {
            delete m_pShaderProgram;
            m_pShaderProgram = nullptr;
        }

        if (m_pCommandBuffer != nullptr)
        {
            delete m_pCommandBuffer;
            m_pCommandBuffer = nullptr;
        }
    }

    void TriangleProgram::onResize(uint32_t width, uint32_t height)
    {
        m_state = ((width == 0) && (height == 0)) ? ProgramState::Paused : ProgramState::Active;
    }

    void TriangleProgram::run()
    {
        draw();
    }

    bool TriangleProgram::shouldRun()
    {
        return m_renderer.shouldRun();
    }

    void TriangleProgram::draw()
    {
        if (m_state != ProgramState::Active)
        {
            return;
        }

        m_renderer.render(m_pCommandBuffer, 1);
    }
}