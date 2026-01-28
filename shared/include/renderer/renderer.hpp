#ifndef LITL_RENDERER_H__
#define LITL_RENDERER_H__

#include "renderer/common.hpp"
#include "renderer/shader/shaderProgram.hpp"
#include "renderer/commandBuffer.hpp"

namespace LITL
{
    class Renderer
    {
    public:

        Renderer();
        ~Renderer();

        bool initialize(uint32_t windowWidth, uint32_t windowHeight, const void* application, const char* applicationName) const;
        bool onResize(int width, int height) const;
        bool shouldRun() const;

        bool recreateShaderPipeline(ShaderProgram** shaders, uint32_t shaderProgramCount) const;
        Shader* createShader(ShaderStage shaderStage) const noexcept;
        ShaderProgram* createShaderProgram() const noexcept;
        CommandBuffer* createCommandBuffer() const noexcept;

        void render(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers) const;

    protected:

    private:

        void cleanup() const;
        void cleanupSwapchain() const;
        void recreateSwapchain() const;

        bool createInstance(const char* applicationName) const;
        bool verifyValidationLayers() const;
        bool createWindowSurface() const;
        bool selectPhysicalDevice() const;
        bool createLogicalDevice() const;
        bool createSwapChain() const;
        bool createCommandPool() const;
        bool createSyncObjects() const;

        bool isRenderReady() const;
        bool acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) const;
        void recordCommandBuffers(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex) const;
        void renderCommandBuffer(CommandBuffer const* pCommandBuffer, uint32_t imageIndex) const;

        RenderContext* m_pContext;
    };
}

#endif 