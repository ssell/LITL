#include "template.hpp"

namespace LITL
{
    TemplateProgram::TemplateProgram()
    {

    }

    bool TemplateProgram::initialize()
    {
        return m_renderer.initialize(1024, 768, this, "Template Demo");
    }

    void TemplateProgram::shutdown()
    {

    }

    void TemplateProgram::onResize(uint32_t width, uint32_t height)
    {
        m_state = ((width == 0) && (height == 0)) ? ProgramState::Paused : ProgramState::Active;
    }

    void TemplateProgram::run()
    {

    }

    bool TemplateProgram::shouldRun()
    {
        return m_renderer.shouldRun();
    }
}