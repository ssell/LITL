#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    class CommandBuffer;

    class Renderer : public LITL::Renderer::Renderer
    {
    public:

        Renderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);
        ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer const&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool initialize() noexcept override;

        uint32_t getFrame() const noexcept override;
        uint32_t getFrameIndex() const noexcept override;

        std::unique_ptr<LITL::Renderer::CommandBuffer> createCommandBuffer() const noexcept override;

        void render(LITL::Renderer::CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers) override;

    protected:

    private:

        void cleanup();
        void cleanupSwapchain();
        void recreateSwapchain();

        bool createInstance();
        bool verifyValidationLayers();
        bool createWindowSurface();
        bool selectPhysicalDevice();
        bool createLogicalDevice();
        bool createSwapChain();
        bool createCommandPool();
        bool createSyncObjects();

        bool isRenderReady() const;
        bool acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex);
        void recordCommandBuffers(CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex);
        void renderCommandBuffer(CommandBuffer* pCommandBuffer, uint32_t imageIndex);

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

    std::unique_ptr<LITL::Renderer::Renderer> createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);
}

#endif