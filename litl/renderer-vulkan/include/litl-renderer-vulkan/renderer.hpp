#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-renderer/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    class Renderer : public LITL::Renderer::Renderer
    {
    public:

        Renderer();
        ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer const&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        bool initialize(char const* title, uint32_t width, uint32_t height) const noexcept override;

    protected:

    private:
    };
}

#endif