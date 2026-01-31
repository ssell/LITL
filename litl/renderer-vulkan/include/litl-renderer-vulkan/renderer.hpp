#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/impl.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    class Renderer : public LITL::Renderer::Renderer
    {
    public:

        Renderer(LITL::Renderer::RendererDescriptor const& rendererDescriptor);
        ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer const&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool initialize() const noexcept override;

    protected:

    private:

        struct Impl;
        Core::ImplPtr<Impl, 64> m_impl;
    };

    LITL::Renderer::Renderer* createVulkanRenderer(LITL::Renderer::RendererDescriptor const& rendererDescriptor);
}

#endif