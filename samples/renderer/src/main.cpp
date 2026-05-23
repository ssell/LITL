#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "litl-renderer/renderer.hpp"
#include "litl-renderer/reflection.hpp"
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

/// <summary>
/// Temporary function to test shader module load, reflection, and creation.
/// </summary>
/// <param name="renderer"></param>
/// <returns></returns>
bool testBuildShader(Renderer* renderer) noexcept
{
    std::cout << "Testing shader construction ..." << std::endl;

    const std::string path = "assets/shaders/spirv/flat.spv";
    const std::string vertEntry = "vertexMain";
    const std::string fragEntry = "fragmentMain";

    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open '" << path << "'" << std::endl;
        return false;
    }

    std::vector<uint8_t> fileBuffer;
    const auto fileSize = static_cast<uint32_t>(file.tellg());
    fileBuffer.resize(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(fileBuffer.data()), fileSize);
    file.close();

    auto vertReflection = reflectSPIRV(vertEntry.c_str(), fileBuffer);
    auto fragReflection = reflectSPIRV(fragEntry.c_str(), fileBuffer);

    if (!vertReflection.has_value() || !fragReflection.has_value())
    {
        // Need both vert + frag
        std::cout << "Failed to reflect vert and/or fragment shader. Vert success = " << vertReflection.has_value() << ", Frag success = " << fragReflection.has_value() << std::endl;
        return false;
    }

    const ShaderModuleDescriptor vertDescriptor{
        .stage = ShaderStage::Vertex,
        .resource = path,
        .entryPoint = vertEntry,
        .bytes = fileBuffer
    };

    const ShaderModuleDescriptor fragDescriptor{
        .stage = ShaderStage::Fragment,
        .resource = path,
        .entryPoint = fragEntry,
        .bytes = fileBuffer
    };

    const auto vertHandle = renderer->createShaderModule(vertDescriptor);
    const auto fragHandle = renderer->createShaderModule(fragDescriptor);

    if (!vertHandle.isValid() || !fragHandle.isValid())
    {
        if (!vertHandle.isValid())
        {
            std::cout << "Failed to create vertex shader module" << std::endl;
        }

        if (!fragHandle.isValid())
        {
            std::cout << "Failed to create fragment shader module" << std::endl;
        }

        return false;
    }

    std::cout << "Successfully created shader modules" << std::endl;

    return true;
}