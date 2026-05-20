#include "litl-core/logging/logging.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"

using namespace litl;

bool createWindow(Window** window)
{
    *window = createVulkanWindow();

    if (window == nullptr)
    {
        return false;
    }

    return (*window)->open("LITL Renderer Only Sample", 1024u, 768u);
}

bool createRenderer(Renderer** renderer, Window* window)
{
    const RendererConfiguration rendererConfig{
        .rendererType = RendererBackendType::Vulkan,
        .framesInFlight = 2
    };

    *renderer = createVulkanRenderer(window, rendererConfig);

    if (renderer == nullptr)
    {
        return false;
    }

    return (*renderer)->build();
}

int main()
{
    Window* window = nullptr;
    Renderer* renderer = nullptr;

    if (createWindow(&window) && createRenderer(&renderer, window))
    {
        CommandBufferHandle commandBuffer = renderer->createCommandBuffer({});

        while (!window->shouldClose())
        {
            if (renderer->beginRender())
            {
                renderer->submitCommands(commandBuffer);
                renderer->endRender();
            }
        }

        renderer->destroyCommandBuffer(commandBuffer);
    }

    destroyVulkanRenderer(renderer);
    destroyVulkanWindow(window);

    return 0;
}