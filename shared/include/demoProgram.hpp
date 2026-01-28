#ifndef LITL_DEMO_PROGRAM_H__
#define LITL_DEMO_PROGRAM_H__

#include "renderer/renderer.hpp"

namespace LITL
{
    enum ProgramState
    {
        Active = 0,
        Paused = 1
    };

    class DemoProgram
    {
    public:

        DemoProgram() : m_state(ProgramState::Active) { }

        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual void onResize(uint32_t width, uint32_t height) = 0;
        virtual void run() = 0;
        virtual bool shouldRun() = 0;

    protected:

        Renderer m_renderer;
        ProgramState m_state;

    private:
    };
}

#endif