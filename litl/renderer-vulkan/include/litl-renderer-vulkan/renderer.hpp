#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
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

        std::unique_ptr<LITL::Renderer::CommandBuffer> createCommandBuffer() const noexcept override;

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
        bool acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) const;
        //void recordCommandBuffers(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex) const;
        //void renderCommandBuffer(CommandBuffer const* pCommandBuffer, uint32_t imageIndex) const;

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

    std::unique_ptr<LITL::Renderer::Renderer> createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);
}

#endif