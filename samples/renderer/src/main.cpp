#include <chrono>
#include <fstream>
#include <iostream>

#include "litl-core/containers/alignedByteBuffer.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"

using namespace litl;

struct Vertex
{
    vec3 position;
    vec3 color;
};

struct PerFrameData
{
    float elapsedTime;
    float deltaTime;
    uint32_t frameCount;
};

struct PushConstants
{
    uint64_t frameDataAddress = 0ull;
};

bool createWindow(Window** window) noexcept;
bool createRenderer(Renderer** renderer, Window* window) noexcept;
color getClearColor(float elapsedSeconds) noexcept;
GraphicsPipelineHandle createTriangleGraphicsPipeline(Renderer* renderer) noexcept;
void beginRender(Renderer* renderer, CommandBufferHandle commandBuffer, color clearColor) noexcept;
void endRender(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept;
BufferHandle createVertexBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept;
BufferHandle createIndexBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept;
BufferHandle createFrameDataBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept;
void updatePerFrameDataBuffer(Renderer* renderer, std::vector<BufferHandle> const& frameBuffers, float elapsedTime, float deltaTime, PushConstants& pushConstants) noexcept;

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
            // Prepare the vertex and index buffers
            BufferHandle vertexBuffer = {};
            BufferHandle indexBuffer = {};
            std::vector<BufferHandle> frameBuffers;
            PushConstants pushConstants{};

            auto lastFrameStart = std::chrono::steady_clock::now();

            while (!window->shouldClose())
            {
                if (renderer->beginRender())
                {
                    const auto frameData = renderer->getFrameData();
                    const auto frameTime = std::chrono::steady_clock::now();
                    const auto elapsedTime = std::chrono::duration<float>(frameTime - start).count();
                    const auto deltaTime = std::chrono::duration<float>(frameTime - lastFrameStart).count();

                    auto commandBuffer = renderer->cmdBeginFrame();

                    if (!vertexBuffer.isValid())
                    {
                        auto scope = renderer->cmdBeginBufferUpload(commandBuffer);

                        vertexBuffer = createVertexBuffer(renderer, commandBuffer);
                        indexBuffer = createIndexBuffer(renderer, commandBuffer);

                        for (auto i = 0; i < frameData.framesInFlight; ++i)
                        {
                            frameBuffers.push_back(createFrameDataBuffer(renderer, commandBuffer));
                        }
                    }

                    updatePerFrameDataBuffer(renderer, frameBuffers, elapsedTime, 0.0f, pushConstants);
                    beginRender(renderer, commandBuffer, getClearColor(elapsedTime));

                    renderer->cmdBindGraphicsPipeline(commandBuffer, graphicsPipelineHandle);
                    renderer->cmdPushConstants(commandBuffer, ShaderStage::Fragment, generic_as_byte_span(&pushConstants, sizeof(PushConstants)));
                    renderer->cmdBindVertexBuffer(commandBuffer, vertexBuffer);
                    renderer->cmdBindIndexBuffer(commandBuffer, indexBuffer);
                    renderer->cmdDraw(commandBuffer, 3, 1, 0, 0);

                    endRender(renderer, commandBuffer);
                    lastFrameStart = frameTime;
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

    GraphicsPipelineDescriptor graphicsPipelineDescriptor{
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
        .vertexInput = VertexInputState{},
        .inputAssembly = InputAssemblyState{},
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

    graphicsPipelineDescriptor.vertexInput.addBinding(VertexBinding{ 0, sizeof(Vertex), VertexInputRate::PerVertex });
    graphicsPipelineDescriptor.vertexInput.addAttribute(VertexAttribute{ 0, 0, DataFormat::RGB32_SFloat, 0 });
    graphicsPipelineDescriptor.vertexInput.addAttribute(VertexAttribute{ 1, 0, DataFormat::RGB32_SFloat, sizeof(vec3) });

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

// -----------------------------------------------------------------------------------------
// Phase 9 Specific Test Code
// -----------------------------------------------------------------------------------------

BufferHandle createVertexBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept
{
    std::array<Vertex, 3> vertices = {
        Vertex {
            .position = { 0.0f, -0.5f, 0.0f },
            .color = { 1.0f, 0.0f, 0.0f }
        },
        Vertex {
            .position = { 0.5f, 0.5f, 0.0f },
            .color = { 0.0f, 1.0f, 0.0f }
        },
        Vertex {
            .position = { -0.5f, 0.5f, 0.0f },
            .color = { 0.0f, 0.0f, 1.0f }
        }
    };

    BufferDescriptor vertexBufferDescriptor{
        .type = BufferTypeFlagBits::VertexBuffer | BufferTypeFlagBits::TransferDest,
        .bytes = sizeof(Vertex) * static_cast<uint32_t>(vertices.size())
    };

    BufferHandle vertexBufferHandle = renderer->createBuffer(vertexBufferDescriptor);

    if (!vertexBufferHandle.isValid())
    {
        std::cout << "Failed to create vertex buffer" << std::endl;
        return {};
    }

    const auto result = renderer->cmdBufferUpload(commandBuffer, as_byte_span(vertices), vertexBufferHandle);

    if (result != RendererResult::Success)
    {
        std::cout << "Failed to write to vertex buffer with result " << static_cast<uint32_t>(result) << std::endl;
    }

    return vertexBufferHandle;
}

BufferHandle createIndexBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept
{
    std::array<uint32_t, 3> indices = { 0, 1, 2 };

    BufferDescriptor indexBufferDescriptor{
        .type = BufferTypeFlagBits::IndexBuffer | BufferTypeFlagBits::TransferDest,
        .bytes = sizeof(uint32_t) * static_cast<uint32_t>(indices.size())
    };

    BufferHandle indexBufferHandle = renderer->createBuffer(indexBufferDescriptor);

    if (!indexBufferHandle.isValid())
    {
        std::cout << "Failed to create index buffer" << std::endl;
        return {};
    }

    const auto result = renderer->cmdBufferUpload(commandBuffer, as_byte_span(indices), indexBufferHandle);

    if (result != RendererResult::Success)
    {
        std::cout << "Failed to write to vertex index with result " << static_cast<uint32_t>(result) << std::endl;
    }

    return indexBufferHandle;
}

BufferHandle createFrameDataBuffer(Renderer* renderer, CommandBufferHandle commandBuffer) noexcept
{
    BufferDescriptor frameDataDescriptor{
        .type = BufferTypeFlagBits::ShaderDeviceAddress,
        .memoryUsage = BufferMemoryUsage::PersistentMap,
        .bytes = sizeof(PerFrameData)
    };

    BufferHandle frameDataBufferHandle = renderer->createBuffer(frameDataDescriptor);

    if (!frameDataBufferHandle.isValid())
    {
        std::cout << "Failed to create frame data buffer" << std::endl;;
        return {};
    }

    return frameDataBufferHandle;
}

void updatePerFrameDataBuffer(Renderer* renderer, std::vector<BufferHandle> const& frameBuffers, float elapsedTime, float deltaTime, PushConstants& pushConstants) noexcept
{
    const auto frameData = renderer->getFrameData();
    const auto perFrameData = PerFrameData{
        .elapsedTime = elapsedTime,
        .deltaTime = deltaTime,
        .frameCount = frameData.frameCount
    };

    auto frameBuffer = frameBuffers[frameData.frameInFlightIndex];
    auto mappedBuffer = renderer->mapBuffer(frameBuffer);

    if ((mappedBuffer.mappedPtr != nullptr) && (mappedBuffer.shaderDeviceAddress != 0ul))
    {
        pushConstants.frameDataAddress = mappedBuffer.shaderDeviceAddress;
        std::memcpy(mappedBuffer.mappedPtr, &perFrameData, sizeof(PerFrameData));
        renderer->unmapBuffer(frameBuffer);
    }
}