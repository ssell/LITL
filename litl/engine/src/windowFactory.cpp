#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-renderer/window.hpp"
#include "litl-engine/windowFactory.hpp"

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
    void injectWindow(ServiceProvider& serviceProvider, PlatformType platformType)
    {
        Window* window = nullptr;

        switch (platformType)
        {
#ifdef LITL_RENDERER_VULKAN
        case PlatformType::Glfw:
            window = createVulkanWindow();
            break;
#endif

#ifdef LITL_RENDERER_D3D12
        case RendererBackendType::D3D12:
            window = createD3D12Window();
            break;
#endif

#ifdef LITL_RENDERER_METAL
        case RendererBackendType::Metal:
            window = createMetalWindow();
            break;
#endif

        default:
            break;
        }

        LITL_FATAL_ASSERT_MSG(window != nullptr, "Failed to create Window");

        serviceProvider.setSingleton<Window, Window>(window);
    }
}