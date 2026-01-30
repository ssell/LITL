#ifndef LITL_VULKAN_RENDERER_H__
#define LITL_VULKAN_RENDERER_H__

#include "litl-core/renderer.hpp"

namespace LITL::Vulkan::Renderer
{
    class Renderer : public LITL::Renderer::Renderer
    {
    public:

        Renderer();
        ~Renderer();

        bool initialize(const char* title, uint32_t width, uint32_t height) const noexcept override;

    protected:

    private:
    };
}

#endif