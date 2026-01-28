#ifndef LITL_TRIANGLE_PROGRAM_H__
#define LITL_TRIANGLE_PROGRAM_H__

#include "demoProgram.hpp"
#include "renderer/renderer.hpp"

namespace LITL
{
    class TemplateProgram : public DemoProgram
    {
    public:

        TemplateProgram();
        TemplateProgram(TemplateProgram const& other) = delete;
        TemplateProgram& operator=(TemplateProgram other) = delete;

        bool initialize() override;
        void shutdown() override;
        void onResize(uint32_t width, uint32_t height) override;

        void run() override;
        bool shouldRun() override;

    protected:

    private:
    };
}

#endif 