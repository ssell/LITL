#include <chrono>
#include <fstream>
#include <iostream>

#include "litl-core/containers/alignedByteBuffer.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"

using namespace litl;

bool createWindow(Window** window) noexcept;
bool createRenderer(Renderer** renderer, Window* window) noexcept;
color getClearColor(float elapsedSeconds) noexcept;
bool testBuildShader(Renderer* renderer) noexcept;

int main()
{
    std::cout << "LITL Renderer Only" << std::endl;

    Window* window = nullptr;
    Renderer* renderer = nullptr;

    if (createWindow(&window) && createRenderer(&renderer, window))
    {
        const auto start = std::chrono::steady_clock::now();

        testBuildShader(renderer);

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

    std::cout << "Shutting down ..." << std::endl;

    destroyVulkanRenderer(renderer);
    destroyVulkanWindow(window);

    std::cout << "Exiting" << std::endl;

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

bool createShaderModule(Renderer* renderer, std::string const& path) noexcept
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open '" << path << "'" << std::endl;
        return false;
    }

    const auto fileSizeBytes = static_cast<size_t>(file.tellg());
    AlignedByteBuffer<4> byteBuffer{ fileSizeBytes };

    file.seekg(0);
    file.read(byteBuffer.as<char>().data(), byteBuffer.size());
    file.close();

    const ShaderModuleDescriptor shaderModuleDescriptor{
        .resource = path,
        .bytes = byteBuffer.as<std::byte>()
    };

    const auto shaderModuleHandle = renderer->createShaderModule(shaderModuleDescriptor);

    if (!shaderModuleHandle.isValid())
    {
        std::cout << "Failed to create shader module" << std::endl;
        return false;
    }

    std::cout << "Successfully created shader modules" << std::endl;

    return true;
}

/// <summary>
/// Temporary function to test shader module load, reflection, and creation.
/// </summary>
/// <param name="renderer"></param>
/// <returns></returns>
bool testBuildShader(Renderer* renderer) noexcept
{
    std::cout << "Testing shader construction ..." << std::endl;

    const std::string resourcePath = "assets/shaders/spirv/flat.spv";

    if (!createShaderModule(renderer, resourcePath))
    {
        return false;
    }

    ShaderModuleHandle shaderHandle = renderer->getShaderModule(resourcePath);

    if (!shaderHandle.isValid())
    {
        std::cout << "Failed to retrieve ShaderModuleHandle" << std::endl;
        return false;
    }

    bool result = renderer->testPipelineLayoutCache(shaderHandle, "vertexMain", shaderHandle, "fragmentMain");

    if (result == false)
    {
        std::cout << "PipelineLayoutCache test failed" << std::endl;
        return false;
    }

    return true;
}