#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/window.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    std::unique_ptr<LITL::Renderer::Renderer> createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor);

    bool build(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    void destroy(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrame(LITL::Renderer::RendererHandle const& litlHandle) noexcept;
    uint32_t getFrameIndex(LITL::Renderer::RendererHandle const& litlHandle) noexcept;

    bool beginRender(LITL::Renderer::RendererHandle const& litlHandle);
    void submitCommands(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers);
    void endRender(LITL::Renderer::RendererHandle const& litlHandle);

    //void render(LITL::Renderer::RendererHandle const& litlHandle, LITL::Renderer::CommandBuffer* pBuffers, uint32_t numCommandBuffers);
    LITL::Renderer::ResourceAllocator* buildResourceAllocator(LITL::Renderer::RendererHandle const& litlHandle) noexcept;

    const LITL::Renderer::RendererOperations VulkanRendererOperations = {
        &build,
        &destroy,
        &getFrame,
        &getFrameIndex,
        &beginRender,
        &submitCommands,
        &endRender,
        &buildResourceAllocator
    };
}

#endif