#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/rendererFactory.hpp"

#ifdef LITL_RENDERER_VULKAN
#include "litl-renderer-vulkan/integration.hpp"
#endif

#ifdef LITL_RENDERER_D3D12
#include "litl-renderer-d3d12/integration.hpp"
#endif

#ifdef LITL_RENDERER_METAL
#include "litl-renderer-metal/integration.hpp"
#endif

namespace litl
{
    void injectRenderer(ServiceProvider& serviceProvider, Window* pWindow, RendererConfiguration const& rendererDescriptor)
    {
        Renderer* renderer = nullptr;

        switch (rendererDescriptor.rendererType)
        {
#ifdef LITL_RENDERER_VULKAN
        case RendererBackendType::Vulkan:
            renderer = createVulkanRenderer(pWindow, rendererDescriptor);
            break;
#endif

#ifdef LITL_RENDERER_D3D12
        case RendererBackendType::D3D12:
            renderer = createD3D12Renderer(pWindow, rendererDescriptor);
            break;
#endif

#ifdef LITL_RENDERER_METAL
        case RendererBackendType::Metal:
            renderer = createMetalRenderer(pWindow, rendererDescriptor);
            break;
#endif

        default:
            break;
        }

        LITL_FATAL_ASSERT_MSG(renderer != nullptr, "Failed to create Renderer");

        serviceProvider.setSingleton<Renderer, Renderer>(renderer);
    }
}