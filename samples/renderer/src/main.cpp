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
GraphicsPipelineHandle createTriangleGraphicsPipeline(Renderer* renderer) noexcept;
void beginRender(Renderer* renderer, CommandBufferHandle commandBuffer, color clearColor) noexcept;
void endRender(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept;
void testShaderHotReload(Renderer* renderer) noexcept;

int main()
{
    std::cout << "LITL Renderer Only" << std::endl;

    Window* window = nullptr;
    Renderer* renderer = nullptr;

    if (createWindow(&window) && createRenderer(&renderer, window))
    {
        const auto start = std::chrono::steady_clock::now();
        const auto graphicsPipelineHandle = createTriangleGraphicsPipeline(renderer);

        if (graphicsPipelineHandle.isValid())
        {
            auto lastTestReload = std::chrono::steady_clock::now();

            while (!window->shouldClose())
            {
                const auto elapsedSeconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();

                if (renderer->beginRender())
                {
                    auto commandBuffer = renderer->cmdBeginFrame();

                    beginRender(renderer, commandBuffer, getClearColor(elapsedSeconds));

                    renderer->cmdBindGraphicsPipeline(commandBuffer, graphicsPipelineHandle);
                    renderer->cmdDraw(commandBuffer, 3, 1, 0, 0);

                    endRender(renderer, commandBuffer);
                }

                if (std::chrono::duration<float>(std::chrono::steady_clock::now() - lastTestReload).count() >= 1.0f)
                {
                    lastTestReload = std::chrono::steady_clock::now();
                    testShaderHotReload(renderer);
                }
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

GraphicsPipelineHandle createTriangleGraphicsPipeline(Renderer* renderer) noexcept
{
    std::cout << "Test Graphics Pipeline Creation ..." << std::endl;;

    const std::string shaderResourcePath = "assets/shaders/spirv/flat.spv";

    if (!createShaderModule(renderer, shaderResourcePath))
    {
        return {};
    }

    ShaderModuleHandle shaderHandle = renderer->getShaderModule(shaderResourcePath);

    if (!shaderHandle.isValid())
    {
        std::cout << "Failed to retrieve ShaderModuleHandle" << std::endl;
        return {};
    }

    const GraphicsPipelineDescriptor graphicsPipelineDescriptor{
        .vertex = PipelineShaderDescriptor {
            .handle = shaderHandle,
            .stage = ShaderStage::Vertex,
            .entryPoint = "vertexMain"
        },
        .fragment = PipelineShaderDescriptor {
            .handle = shaderHandle,
            .stage = ShaderStage::Fragment,
            .entryPoint = "fragmentMain"
        },
        .renderTargets = RenderTargetFormats {
            .colorAttachments = { renderer->getSwapchainImageFormat() },
            .colorAttachmentCount = 1
        },
        .vertexInput = VertexInputState{},                      // Vertices hardcoded in the vertex shader for now
        .inputAssembly = InputAssemblyState{},                  // Vertices hardcoded in the vertex shader for now
        .tessellation = std::nullopt,                           // No tessellation
        .rasterization = RasterizationState{},                  // Default rasterization state OK for this test
        .multisample = MultisampleState{},                      // Default multisample state (no multisampling) for this test
        .depthStencil = DepthStencilState{},                    // Default depth-stencil settings for this test
        .colorBlend = ColorBlendState{
            .logicOpEnabled = false,
            .colorAttachmentBlendStates = {
                ColorBlendAttachmentState {
                    .attachmentBlendEnabled = false
                }
            }
        },
        .dynamicState = DynamicStateMask{},                     // Default dynamic state (viewport + stencil) for this test
        .specializationConstants = SpecializationConstants{}    // Default specialization constants (none) for this test
    };

    const GraphicsPipelineHandle handle = renderer->createGraphicsPipeline(graphicsPipelineDescriptor);

    if (handle.isValid())
    {
        std::cout << "Successfully create GraphicsPipelineHandle" << std::endl;
    }
    else
    {
        std::cout << "Failed to created GraphicsPipelineHandle" << std::endl;
    }

    return handle;
}

void beginRender(Renderer* renderer, CommandBufferHandle commandBuffer, color clearColor) noexcept
{
    const BeginRenderCommand beginRenderCommand{
        .color = ColorAttachmentDescriptor {
            .clearColor = clearColor
        }
    };

    const SetViewportAndScissorCommand setViewportScissorCommand{
        .setViewport = SetViewportCommand {
            .region = {
                .offset = { 0.0f, 0.0f },
                .extents = { 1.0f, 1.0f }           // normalized
            },
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        },
        .setScissor = SetScissorCommand {
            .region = {
                .offset = { 0.0f, 0.0f },
                .extents = { 1.0f, 1.0f }           // normalized
            }
        }
    };

    renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierUndefinedToColor);
    renderer->cmdBeginRender(commandBuffer, beginRenderCommand);
    renderer->cmdSetViewportAndScissor(commandBuffer, setViewportScissorCommand);
}

void endRender(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept
{
    renderer->cmdEndRender(commandBuffer);
    renderer->cmdPipelineBarrier(commandBuffer, PipelineBarrierColorToPresent);
    renderer->cmdEnd(commandBuffer);

    renderer->submitCommands(commandBuffer);
    renderer->endRender();
}

void testShaderHotReload(Renderer* renderer) noexcept
{
    const std::string shaderResourcePath = "assets/shaders/spirv/flat.spv";
    std::ifstream file(shaderResourcePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        std::cout << "Failed to open '" << shaderResourcePath << "'" << std::endl;
        return;
    }

    const auto fileSizeBytes = static_cast<size_t>(file.tellg());
    AlignedByteBuffer<4> byteBuffer{ fileSizeBytes };

    file.seekg(0);
    file.read(byteBuffer.as<char>().data(), byteBuffer.size());
    file.close();

    const ShaderModuleDescriptor shaderModuleDescriptor{
        .resource = shaderResourcePath,
        .bytes = byteBuffer.as<std::byte>()
    };

    renderer->reloadShaderModule(shaderModuleDescriptor);
}