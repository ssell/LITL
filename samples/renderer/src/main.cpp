#include <chrono>

#include "litl-renderer/renderer.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"

using namespace litl;

bool createWindow(Window** window) noexcept;
bool createRenderer(Renderer** renderer, Window* window) noexcept;
color getClearColor(float elapsedSeconds) noexcept;

int main()
{
    Window* window = nullptr;
    Renderer* renderer = nullptr;

    if (createWindow(&window) && createRenderer(&renderer, window))
    {
        const auto start = std::chrono::steady_clock::now();

        while (!window->shouldClose())
        {
            const auto elapsedSeconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

            if (renderer->beginRender())
            {
                auto commandBuffer = renderer->cmdBeginFrame();
                renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierUndefinedToColor);
                renderer->cmdBeginRender(commandBuffer, { .color = { .clearColor = getClearColor(elapsedSeconds) }});

                // ... add other render commands ...

                renderer->cmdEndRender(commandBuffer);
                renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierColorToPresent);
                renderer->cmdEnd(commandBuffer);

                renderer->submitCommands(commandBuffer);
                renderer->endRender();
            }
        }
    }

    destroyVulkanRenderer(renderer);
    destroyVulkanWindow(window);

    return 0;
}

bool createWindow(Window** window) noexcept
{
    *window = createVulkanWindow();

    if (window == nullptr)
    {
        return false;
    }

    return (*window)->open("LITL Renderer Only Sample", 1024u, 768u);
}

bool createRenderer(Renderer** renderer, Window* window) noexcept
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

color getClearColor(float elapsedSeconds) noexcept
{
    return {
        (static_cast<float>(std::cos(elapsedSeconds)) + 1.0f) * 0.5f,
        (static_cast<float>(std::sin(elapsedSeconds)) + 1.0f) * 0.5f,
        0.5f
    };
}