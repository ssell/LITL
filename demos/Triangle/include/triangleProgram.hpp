#ifndef LITL_TRIANGLE_PROGRAM_H__
#define LITL_TRIANGLE_PROGRAM_H__

#include "demoProgram.hpp"
#include "renderer/renderer.hpp"

namespace LITL
{
    class TriangleProgram : public DemoProgram
    {
    public:

        TriangleProgram();
        TriangleProgram(TriangleProgram const& other) = delete;
        TriangleProgram& operator=(TriangleProgram other) = delete;

        bool initialize() override;
        void shutdown() override;
        void onResize(uint32_t width, uint32_t height) override;
        void run() override;
        bool shouldRun() override;


    protected:

    private:

        void draw();

        Renderer::ShaderProgram* m_pShaderProgram;
        Renderer::CommandBuffer* m_pCommandBuffer;
    };
}

#endif 