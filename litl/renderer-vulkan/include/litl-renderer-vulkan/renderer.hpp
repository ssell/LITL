#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/impl.hpp"
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

        bool initialize() const noexcept override;

    protected:

    private:

        void cleanup() const;
        void cleanupSwapchain() const;
        void recreateSwapchain() const;

        bool createInstance() const;
        bool verifyValidationLayers() const;
        bool createWindowSurface() const;
        bool selectPhysicalDevice() const;
        bool createLogicalDevice() const;
        bool createSwapChain() const;
        bool createCommandPool() const;
        bool createSyncObjects() const;

        bool isRenderReady() const;
        bool acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) const;
        //void recordCommandBuffers(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex) const;
        //void renderCommandBuffer(CommandBuffer const* pCommandBuffer, uint32_t imageIndex) const;

        struct Impl;
        Core::ImplPtr<Impl, 64> m_impl;
    };

    LITL::Renderer::Renderer* createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);
}

#endif