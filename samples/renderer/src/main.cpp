#include <chrono>
#include <fstream>
#include <iostream>

#include "litl-core/containers/alignedByteBuffer.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-core/math/types.hpp"
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
};

struct PerCameraData
{
    mat4 viewMatrix;
    mat4 projMatrix;
    mat4 viewProjMatrix;
};

struct PushConstants
{
    uint64_t frameDataAddress = 0ull;
};

/// <summary>
/// Everything used by the Renderer sample.
/// </summary>
struct SampleRenderState
{
    // -------------------------------------------------------------------------------------
    // Timing
    // -------------------------------------------------------------------------------------

    float elapsedTime = 0.0f;
    float deltaTime = 0.0f;

    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point frameStartTime;
    std::chrono::steady_clock::time_point lastFrameTime;

    // -------------------------------------------------------------------------------------
    // Renderer Resources
    // -------------------------------------------------------------------------------------

    Window* window = nullptr;
    Renderer* renderer = nullptr;
    uint32_t framesInFlight = 0u;
    FrameData frameData{};
    CommandBufferHandle commandBuffer{};
    GraphicsPipelineHandle graphicsPipeline{};

    // -------------------------------------------------------------------------------------
    // Render Data
    // -------------------------------------------------------------------------------------

    PerFrameData perFrameData{};
    PerCameraData perCameraData{};
    PushConstants pushConstants{};

    // -------------------------------------------------------------------------------------
    // Buffers
    // -------------------------------------------------------------------------------------

    BufferHandle vertexBuffer{};
    BufferHandle indexBuffer{};
    std::vector<BufferHandle> frameDataBuffers;
    std::vector<BufferHandle> cameraDataBuffers;

    // -------------------------------------------------------------------------------------
    // Textures
    // -------------------------------------------------------------------------------------

    TextureHandle stripedTexture{};
};

bool createWindow(SampleRenderState& sample) noexcept;
bool createRenderer(SampleRenderState& sample) noexcept;
bool createTriangleGraphicsPipeline(SampleRenderState& sample) noexcept;
void updateTiming(SampleRenderState& sample) noexcept;
void beginRender(SampleRenderState& sample) noexcept;
void endRender(SampleRenderState& sample) noexcept;
bool prepareSample(SampleRenderState& sample) noexcept;
bool createVertexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept;
bool createIndexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept;
bool createFrameDataBuffers(SampleRenderState& sample) noexcept;
bool createFrameDataBuffer(SampleRenderState& sample) noexcept;
void updatePerFrameDataBuffer(SampleRenderState& sample) noexcept;
bool createCameraDataBuffers(SampleRenderState& sample) noexcept;
bool createCameraDataBuffer(SampleRenderState& sample) noexcept;
void updatePerCameraDataBuffer(SampleRenderState& sample) noexcept;
bool createTexture(SampleRenderState& sample) noexcept;

int main()
{
    std::cout << "LITL Renderer Only" << std::endl;

    SampleRenderState sample{};

    if (createWindow(sample) && createRenderer(sample))
    {
        sample.startTime = std::chrono::steady_clock::now();

        if (createTriangleGraphicsPipeline(sample) && prepareSample(sample))
        {
            sample.lastFrameTime = std::chrono::steady_clock::now();

            while (!sample.window->shouldClose())
            {
                if (sample.renderer->beginRender())
                {
                    updateTiming(sample);

                    sample.frameData = sample.renderer->getFrameData();
                    sample.commandBuffer = sample.renderer->cmdBeginFrame();

                    updatePerFrameDataBuffer(sample);
                    updatePerCameraDataBuffer(sample);

                    beginRender(sample);

                    sample.renderer->cmdBindGraphicsPipeline(sample.commandBuffer, sample.graphicsPipeline);
                    sample.renderer->cmdPushConstants(sample.commandBuffer, ShaderStage::Fragment, generic_as_byte_span(&sample.pushConstants, sizeof(PushConstants)));
                    sample.renderer->cmdBindGraphicsBuffer(sample.commandBuffer, sample.cameraDataBuffers[sample.frameData.frameInFlightIndex], "_cameraData"_sid);
                    sample.renderer->cmdBindVertexBuffer(sample.commandBuffer, sample.vertexBuffer);
                    sample.renderer->cmdBindIndexBuffer(sample.commandBuffer, sample.indexBuffer);
                    sample.renderer->cmdDraw(sample.commandBuffer, 3, 1, 0, 0);

                    endRender(sample);
                    sample.lastFrameTime = sample.frameStartTime;
                }
            }
        }
    }

    std::cout << "Shutting down ..." << std::endl;

    destroyVulkanRenderer(sample.renderer);
    destroyVulkanWindow(sample.window);

    std::cout << "Exiting" << std::endl;

    return 0;
}

bool createWindow(SampleRenderState& sample) noexcept
{
    sample.window = createVulkanWindow();

    if (sample.window == nullptr)
    {
        return false;
    }

    return sample.window->open("LITL Renderer Only Sample", 1024u, 768u);
}

bool createRenderer(SampleRenderState& sample) noexcept
{
    const RendererConfiguration rendererConfig{
        .rendererType = RendererBackendType::Vulkan,
        .framesInFlight = 2
    };

    sample.renderer = createVulkanRenderer(sample.window, rendererConfig);

    if (sample.renderer == nullptr)
    {
        return false;
    }

    sample.framesInFlight = sample.renderer->getFrameData().framesInFlight;

    return sample.renderer->build();
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

    return true;
}

bool createTriangleGraphicsPipeline(SampleRenderState& sample) noexcept
{
    const std::string shaderResourcePath = "assets/shaders/spirv/flat.spv";

    if (!createShaderModule(sample.renderer, shaderResourcePath))
    {
        return {};
    }

    ShaderModuleHandle shaderHandle = sample.renderer->getShaderModule(shaderResourcePath);

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
            .colorAttachments = { sample.renderer->getSwapchainImageFormat() },
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

    sample.graphicsPipeline = sample.renderer->createGraphicsPipeline(graphicsPipelineDescriptor);

    if (!sample.graphicsPipeline.isValid())
    {
        std::cout << "Failed to create GraphicsPipelineHandle" << std::endl;
        return false;
    }

    return true;
}

void updateTiming(SampleRenderState& sample) noexcept
{
    sample.frameStartTime = std::chrono::steady_clock::now();
    sample.elapsedTime = std::chrono::duration<float>(sample.frameStartTime - sample.startTime).count();
    sample.deltaTime = std::chrono::duration<float>(sample.frameStartTime - sample.lastFrameTime).count();
}

void beginRender(SampleRenderState& sample) noexcept
{
    const BeginRenderCommand beginRenderCommand{
        .color = ColorAttachmentDescriptor {
            .clearColor = color(0.05f, 0.05f, 0.075f, 1.0f)
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

    sample.renderer->cmdPipelineBarrier(sample.commandBuffer, PipelineBarrierUndefinedToColor);
    sample.renderer->cmdBeginRender(sample.commandBuffer, beginRenderCommand);
    sample.renderer->cmdSetViewportAndScissor(sample.commandBuffer, setViewportScissorCommand);
}

void endRender(SampleRenderState& sample) noexcept
{
    sample.renderer->cmdEndRender(sample.commandBuffer);
    sample.renderer->cmdPipelineBarrier(sample.commandBuffer, PipelineBarrierColorToPresent);
    sample.renderer->cmdEnd(sample.commandBuffer);

    sample.renderer->submitCommands(sample.commandBuffer);
    sample.renderer->endRender();
}

bool prepareSample(SampleRenderState& sample) noexcept
{
    if (sample.vertexBuffer.isValid())
    {
        return true;
    }


    bool success = false;

    // Create the buffers prior to the first frame using a transient, single-shot command buffer.
    auto scopedCommandBuffer = sample.renderer->createScopedCommandBuffer();
    auto commandBufferHandle = scopedCommandBuffer.get();

    if (createVertexBuffer(sample, commandBufferHandle) &&
        createIndexBuffer(sample, commandBufferHandle) &&
        createFrameDataBuffers(sample) &&
        createCameraDataBuffers(sample) &&
        createTexture(sample))
    {
        success = true;
    }
    else
    {
        std::cout << "Failed to prepare resources for Renderer Sample" << std::endl;
    };

    sample.renderer->cmdBufferFlush(commandBufferHandle);

    return success;
}

bool createVertexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept
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

    sample.vertexBuffer = sample.renderer->createBuffer(vertexBufferDescriptor);

    if (!sample.vertexBuffer.isValid())
    {
        std::cout << "Failed to create vertex buffer" << std::endl;
        return false;
    }

    const auto result = sample.renderer->cmdBufferUpload(commandBuffer, as_byte_span(vertices), sample.vertexBuffer);

    if (result != RendererResult::Success)
    {
        std::cout << "Failed to write to vertex buffer with result " << static_cast<uint32_t>(result) << std::endl;
        return false;
    }

    return true;
}

bool createIndexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept
{
    std::array<uint32_t, 3> indices = { 0, 1, 2 };

    BufferDescriptor indexBufferDescriptor{
        .type = BufferTypeFlagBits::IndexBuffer | BufferTypeFlagBits::TransferDest,
        .bytes = sizeof(uint32_t) * static_cast<uint32_t>(indices.size())
    };

    sample.indexBuffer = sample.renderer->createBuffer(indexBufferDescriptor);

    if (!sample.indexBuffer.isValid())
    {
        std::cout << "Failed to create index buffer" << std::endl;
        return {};
    }

    const auto result = sample.renderer->cmdBufferUpload(commandBuffer, as_byte_span(indices), sample.indexBuffer);

    if (result != RendererResult::Success)
    {
        std::cout << "Failed to write to vertex index with result " << static_cast<uint32_t>(result) << std::endl;
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------
// Frame Data Buffer
// -----------------------------------------------------------------------------------------

// Per-frame data is provided via a persistently-mapped multi-buffer buffer (one per frame-in-flight).
// The shader samples the buffer using BDA via the buffer address being provided in the Push Constant.

bool createFrameDataBuffers(SampleRenderState& sample) noexcept
{
    for (auto i = 0; i < sample.framesInFlight; ++i)
    {
        if (!createFrameDataBuffer(sample))
        {
            return false;
        }
    }

    return true;
}

bool createFrameDataBuffer(SampleRenderState& sample) noexcept
{
    BufferDescriptor frameBufferDescriptor{
        .type = BufferTypeFlagBits::ShaderDeviceAddress,
        .memoryUsage = BufferMemoryUsage::PersistentMap,
        .bytes = sizeof(PerFrameData)
    };

    BufferHandle frameBufferHandle = sample.renderer->createBuffer(frameBufferDescriptor);

    if (!frameBufferHandle.isValid())
    {
        std::cout << "Failed to create frame data buffer" << std::endl;
        return false;
    }

    sample.frameDataBuffers.push_back(frameBufferHandle);

    return true;
}

void updatePerFrameDataBuffer(SampleRenderState& sample) noexcept
{
    sample.perFrameData.elapsedTime = sample.elapsedTime;
    sample.perFrameData.deltaTime = sample.deltaTime;

    MappedBuffer mappedBuffer{};

    auto frameBuffer = sample.frameDataBuffers[sample.frameData.frameInFlightIndex];

    if (sample.renderer->mapBuffer(frameBuffer, mappedBuffer) == RendererResult::Success)
    {
        sample.pushConstants.frameDataAddress = mappedBuffer.shaderDeviceAddress;
        std::memcpy(mappedBuffer.mappedPtr, &sample.perFrameData, sizeof(PerFrameData));
        sample.renderer->unmapBuffer(frameBuffer);
    }
}

// -----------------------------------------------------------------------------------------
// Camera Data Buffer
// -----------------------------------------------------------------------------------------

bool createCameraDataBuffers(SampleRenderState& sample) noexcept
{
    for (auto i = 0; i < sample.framesInFlight; ++i)
    {
        if (!createCameraDataBuffer(sample))
        {
            return false;
        }
    }

    return true;
}

bool createCameraDataBuffer(SampleRenderState& sample) noexcept
{
    BufferDescriptor cameraBufferDescriptor{
        .type = BufferTypeFlagBits::UniformBuffer,
        .memoryUsage = BufferMemoryUsage::PersistentMap,
        .bytes = sizeof(PerCameraData)
    };

    BufferHandle cameraBufferHandle = sample.renderer->createBuffer(cameraBufferDescriptor);

    if (!cameraBufferHandle.isValid())
    {
        std::cout << "Failed to create camera data buffer" << std::endl;;
        return false;
    }

    sample.cameraDataBuffers.push_back(cameraBufferHandle);

    return true;
}

void updatePerCameraDataBuffer(SampleRenderState& sample) noexcept
{
    MappedBuffer mappedBuffer{};

    auto cameraBuffer = sample.cameraDataBuffers[sample.frameData.frameInFlightIndex];

    if (sample.renderer->mapBuffer(cameraBuffer, mappedBuffer) == RendererResult::Success)
    {
        vec3 cameraPos{ 0.0f, 0.0f, -2.5f };
        cameraPos.z() += cos(sample.elapsedTime);

        sample.perCameraData.projMatrix = mat4::perspective(degreesToRadians(60.0f), sample.window->getAspectRatio(), 0.0f, 10.0f);
        sample.perCameraData.viewMatrix = mat4::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3::up());
        sample.perCameraData.viewProjMatrix = sample.perCameraData.projMatrix * sample.perCameraData.viewMatrix;

        std::memcpy(mappedBuffer.mappedPtr, &sample.perCameraData, sizeof(PerCameraData));
        sample.renderer->unmapBuffer(cameraBuffer);
    }
}

// -----------------------------------------------------------------------------------------
// Texture
// -----------------------------------------------------------------------------------------

bool createTexture(SampleRenderState& sample) noexcept
{
    sample.stripedTexture = sample.renderer->createTexture(TextureDescriptor{
        .width = 256,
        .height = 256,
        .format = DataFormat::RGBA8_SRGB,
        .name = "Striped Texture"
    });

    if (!sample.stripedTexture.isValid())
    {
        std::cout << "Failed to create striped texture" << std::endl;
        return false;
    }

    return true;
}