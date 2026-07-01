/**
 * ----------------------------------------------------------------------------------------- 
 * ---- LITL Renderer Sample
 * -----------------------------------------------------------------------------------------
 * 
 * This is a sample of using the litl-renderer.
 * 
 * While the litl-renderer is intended to be abstracted away and rarely used directly outside
 * of the litl-engine library, there may still be use-cases for interacting with it directly.
 */

#include <chrono>
#include <fstream>

#include "litl-core/logging/logging.hpp"
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
    vec2 uv;
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

    TextureHandle texture{};
    SamplerHandle sampler{};
};

bool setupSample(SampleRenderState& sample) noexcept;
void cleanupSample(SampleRenderState& sample) noexcept;
bool createWindow(SampleRenderState& sample) noexcept;
bool createRenderer(SampleRenderState& sample) noexcept;
bool createGraphicsPipeline(SampleRenderState& sample) noexcept;
void updateTiming(SampleRenderState& sample) noexcept;
void beginRender(SampleRenderState& sample) noexcept;
void endRender(SampleRenderState& sample) noexcept;
bool createResources(SampleRenderState& sample) noexcept;
bool createVertexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept;
bool createIndexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept;
bool createFrameDataBuffers(SampleRenderState& sample) noexcept;
bool createFrameDataBuffer(SampleRenderState& sample) noexcept;
void updatePerFrameDataBuffer(SampleRenderState& sample) noexcept;
bool createCameraDataBuffers(SampleRenderState& sample) noexcept;
bool createCameraDataBuffer(SampleRenderState& sample) noexcept;
void updatePerCameraDataBuffer(SampleRenderState& sample) noexcept;
bool createTexture(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept;
bool createSampler(SampleRenderState& sample) noexcept;

int main()
{
    litl::Logger::initialize("litl-renderer-sample", true, false);

    logInfo("LITL Renderer Only");

    SampleRenderState sample{};

    if (setupSample(sample))
    {
        sample.startTime = std::chrono::steady_clock::now();
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
                sample.renderer->cmdBindBuffer(sample.commandBuffer, sample.cameraDataBuffers[sample.frameData.frameInFlightIndex], "_cameraData"_sid, true);
                sample.renderer->cmdBindVertexBuffer(sample.commandBuffer, sample.vertexBuffer);
                sample.renderer->cmdBindIndexBuffer(sample.commandBuffer, sample.indexBuffer);
                sample.renderer->cmdBindSampler(sample.commandBuffer, sample.sampler, "_texture_sampler"_sid, true);
                sample.renderer->cmdBindTexture(sample.commandBuffer, sample.texture, "_texture"_sid, true);
                sample.renderer->cmdDraw(sample.commandBuffer, 3, 1, 0, 0);

                endRender(sample);

                sample.lastFrameTime = sample.frameStartTime;
            }
        }
    }

    cleanupSample(sample);
    logInfo("Exiting");

    return 0;
}

/// <summary>
/// Prepares the window, renderer, and various resources used by the sample.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
bool setupSample(SampleRenderState& sample) noexcept
{
    return createWindow(sample) && createRenderer(sample) && createResources(sample);
}

/// <summary>
/// Cleans up all resources used by the sample.
/// </summary>
/// <param name="sample"></param>
void cleanupSample(SampleRenderState& sample) noexcept
{
    logInfo("Shutting down ...");

    if (sample.renderer != nullptr)
    {
        // This will automatically cleanup all outstanding resources.
        destroyVulkanRenderer(sample.renderer);
    }

    destroyVulkanWindow(sample.window);
}

/// <summary>
/// Creates the window that we will render into.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
bool createWindow(SampleRenderState& sample) noexcept
{
    sample.window = createVulkanWindow();

    if (sample.window == nullptr)
    {
        return false;
    }

    return sample.window->open("LITL Renderer Only Sample", 1024u, 768u);
}

/// <summary>
/// Creates the Vulkan renderer.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
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

/// <summary>
/// Creates the shader module for the shader at the specified path.
/// </summary>
/// <param name="renderer"></param>
/// <param name="path"></param>
/// <returns></returns>
bool createShaderModule(Renderer* renderer, std::string const& path) noexcept
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        logError("Failed to open '", path, "'");
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
        logError("Failed to created shader module");
        return false;
    }

    return true;
}

/// <summary>
/// Updates timing data for the current frame.
/// </summary>
/// <param name="sample"></param>
void updateTiming(SampleRenderState& sample) noexcept
{
    sample.frameStartTime = std::chrono::steady_clock::now();
    sample.elapsedTime = std::chrono::duration<float>(sample.frameStartTime - sample.startTime).count();
    sample.deltaTime = std::chrono::duration<float>(sample.frameStartTime - sample.lastFrameTime).count();
}

/// <summary>
/// Issues the commands to start rendering a new frame.
/// </summary>
/// <param name="sample"></param>
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

/// <summary>
/// Issues the commands to end rendering the current frame.
/// </summary>
/// <param name="sample"></param>
void endRender(SampleRenderState& sample) noexcept
{
    sample.renderer->cmdEndRender(sample.commandBuffer);
    sample.renderer->cmdPipelineBarrier(sample.commandBuffer, PipelineBarrierColorToPresent);
    sample.renderer->cmdEnd(sample.commandBuffer);

    sample.renderer->submitCommands(sample.commandBuffer);
    sample.renderer->endRender();
}

/// <summary>
/// Creates all resources used by the sample.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
bool createResources(SampleRenderState& sample) noexcept
{
    if (sample.vertexBuffer.isValid())
    {
        return true;
    }


    bool success = false;

    // Create the buffers prior to the first frame using a transient, single-shot command buffer.
    auto scopedCommandBuffer = sample.renderer->createScopedCommandBuffer();
    auto commandBufferHandle = scopedCommandBuffer.get();

    if (createGraphicsPipeline(sample) &&
        createVertexBuffer(sample, commandBufferHandle) &&
        createIndexBuffer(sample, commandBufferHandle) &&
        createFrameDataBuffers(sample) &&
        createCameraDataBuffers(sample) &&
        createTexture(sample, commandBufferHandle) &&
        createSampler(sample))
    {
        success = true;
    }
    else
    {
        logError("Failed to prepare resources for Renderer Sample");
    };

    sample.renderer->cmdBufferFlush(commandBufferHandle);

    return success;
}

/// <summary>
/// Creates the graphics pipeline resource for the sample shader.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
bool createGraphicsPipeline(SampleRenderState& sample) noexcept
{
    const std::string shaderResourcePath = "assets/shaders/spirv/triangleSample.spv";

    if (!createShaderModule(sample.renderer, shaderResourcePath))
    {
        return { };
    }

    ShaderModuleHandle shaderHandle = sample.renderer->getShaderModule(shaderResourcePath);

    if (!shaderHandle.isValid())
    {
        logError("Failed to retrieve ShaderModuleHandle");
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
        .inputAssembly = InputAssemblyState{},                  // Default input assembly for this sample
        .tessellation = std::nullopt,                           // No tessellation
        .rasterization = RasterizationState{},                  // Default rasterization state for this sample
        .multisample = MultisampleState{},                      // Default multisample state (no multisampling) for this sample
        .depthStencil = DepthStencilState{},                    // Default depth-stencil settings for this sample
        .colorBlend = ColorBlendState{
            .logicOpEnabled = false,
            .colorAttachmentBlendStates = {
                ColorBlendAttachmentState {
                    .attachmentBlendEnabled = false
                }
            }
        },
        .dynamicState = DynamicStateMask{},                     // Default dynamic state (viewport + stencil) for this sample
        .specializationConstants = SpecializationConstants{}    // Default specialization constants (none) for this sample
    };

    graphicsPipelineDescriptor.vertexInput.addBinding(VertexBinding{ 0, sizeof(Vertex), VertexInputRate::PerVertex });

    uint32_t offset = 0u;
    graphicsPipelineDescriptor.vertexInput.addAttribute<vec3>(VertexAttribute{ 0, 0, DataFormat::RGB32_SFloat }, offset);  // position
    graphicsPipelineDescriptor.vertexInput.addAttribute<vec3>(VertexAttribute{ 1, 0, DataFormat::RGB32_SFloat }, offset);  // color
    graphicsPipelineDescriptor.vertexInput.addAttribute<vec2>(VertexAttribute{ 2, 0, DataFormat::RG32_SFloat }, offset);   // uv

    sample.graphicsPipeline = sample.renderer->createGraphicsPipeline(graphicsPipelineDescriptor);

    if (!sample.graphicsPipeline.isValid())
    {
        logError("Failed to create GraphicsPipelineHandle");
        return false;
    }

    return true;
}

/// <summary>
/// Creates the vertex buffer composed of three vertices to form the sample triangle.
/// </summary>
/// <param name="sample"></param>
/// <param name="commandBuffer"></param>
/// <returns></returns>
bool createVertexBuffer(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept
{
    // litl uses clockwise winding by default (can be changed in the graphics pipeline setup)
    std::array<Vertex, 3> vertices = {
        Vertex {                                        // left
            .position = { -0.5f, 0.0f, 0.0f },
            .color = { 0.0f, 1.0f, 0.0f },
            .uv = { 0.0f, 0.0f }
        },
        Vertex {                                        // top
            .position = { 0.0f, 1.0f, 0.0f },
            .color = { 1.0f, 0.0f, 0.0f },
            .uv = { 0.5f, 1.0f }
        },
        Vertex {                                        // right
            .position = { 0.5f, 0.0f, 0.0f },
            .color = { 0.0f, 0.0f, 1.0f },
            .uv = { 1.0f, 0.0f }
        }
    };

    BufferDescriptor vertexBufferDescriptor{
        .type = BufferTypeFlagBits::VertexBuffer | BufferTypeFlagBits::TransferDest,
        .bytes = sizeof(Vertex) * static_cast<uint32_t>(vertices.size())
    };

    sample.vertexBuffer = sample.renderer->createBuffer(vertexBufferDescriptor);

    if (!sample.vertexBuffer.isValid())
    {
        logError("Failed to create vertex buffer");
        return false;
    }

    const auto result = sample.renderer->cmdBufferUpload(commandBuffer, as_byte_span(vertices), sample.vertexBuffer);

    if (result != RendererResult::Success)
    {
        logError("Failed to write to vertex buffer with result ", static_cast<uint32_t>(result));
        return false;
    }

    return true;
}

/// <summary>
/// Creates the index buffer composed of three indices to form the sample triangle.
/// </summary>
/// <param name="sample"></param>
/// <param name="commandBuffer"></param>
/// <returns></returns>
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
        logError("Failed to create index buffer");
        return {};
    }

    const auto result = sample.renderer->cmdBufferUpload(commandBuffer, as_byte_span(indices), sample.indexBuffer);

    if (result != RendererResult::Success)
    {
        logError("Failed to write to vertex index with result ",static_cast<uint32_t>(result));
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------------------
// Frame Data Buffer
// -----------------------------------------------------------------------------------------

/// <summary>
/// Per-frame data is provided via a persistently-mapped multi-buffer buffer (one per frame-in-flight).
/// The shader samples the buffer using BDA via the buffer address being provided in the Push Constant.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
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

/// <summary>
/// Creates a frame data buffer for an individual frame-in-flight.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
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
        logError("Failed to create frame data buffer");
        return false;
    }

    sample.frameDataBuffers.push_back(frameBufferHandle);

    return true;
}

/// <summary>
/// Updates the frame data buffer for the current frame.
/// </summary>
/// <param name="sample"></param>
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

/// <summary>
/// Creates a camera data buffer for each frame-in-flight.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
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

/// <summary>
/// Creates a camera data buffer for an individual frame-in-flight.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
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
        logError("Failed to create camera data buffer");
        return false;
    }

    sample.cameraDataBuffers.push_back(cameraBufferHandle);

    return true;
}

/// <summary>
/// Updates the camera data buffer for the current frame.
/// </summary>
/// <param name="sample"></param>
void updatePerCameraDataBuffer(SampleRenderState& sample) noexcept
{
    MappedBuffer mappedBuffer{};

    auto cameraBuffer = sample.cameraDataBuffers[sample.frameData.frameInFlightIndex];

    if (sample.renderer->mapBuffer(cameraBuffer, mappedBuffer) == RendererResult::Success)
    {
        vec3 cameraPos{ 0.0f, 0.5f, -2.5f };
        cameraPos.z() += cos(sample.elapsedTime);

        sample.perCameraData.projMatrix = mat4::perspective(degreesToRadians(60.0f), sample.window->getAspectRatio(), 0.0f, 10.0f);
        sample.perCameraData.viewMatrix = mat4::lookAt(cameraPos, vec3(0.0f, 0.5f, 0.0f), vec3::up());
        sample.perCameraData.viewProjMatrix = sample.perCameraData.projMatrix * sample.perCameraData.viewMatrix;

        std::memcpy(mappedBuffer.mappedPtr, &sample.perCameraData, sizeof(PerCameraData));
        sample.renderer->unmapBuffer(cameraBuffer);
    }
}

// -----------------------------------------------------------------------------------------
// Texture
// -----------------------------------------------------------------------------------------

/// <summary>
/// Creates the sample texture that is applied to the triangle.
/// </summary>
/// <param name="sample"></param>
/// <param name="commandBuffer"></param>
/// <returns></returns>
bool createTexture(SampleRenderState& sample, CommandBufferHandle commandBuffer) noexcept
{
    sample.texture = sample.renderer->createTexture(TextureDescriptor{
        .width = 3,
        .height = 3,
        .format = DataFormat::RGBA32_SFloat,
        .name = "Sample Texture"
    });

    if (!sample.texture.isValid())
    {
        logError("Failed to create sample texture");
        return false;
    }

    std::array<color, 9> pixels = {
        colors::Blue, colors::Blue, colors::Blue,           // bottom
        colors::Green, colors::Green, colors::Green,
        colors::Red, colors::Red, colors::Red               // top
    };

    const auto result = sample.renderer->cmdTextureUpload(commandBuffer, as_byte_span(pixels), sample.texture);

    if (result != RendererResult::Success)
    {
        logError("Failed to write to sample texture with result ", static_cast<uint32_t>(result));
        return false;
    }

    return true;
}

/// <summary>
/// Creates the sampler used to sample the texture.
/// </summary>
/// <param name="sample"></param>
/// <returns></returns>
bool createSampler(SampleRenderState& sample) noexcept
{
    // A basic nearest/point sampler to maintain the crisp lines in the 3x3 texture.
    sample.sampler = sample.renderer->createSampler(SamplerDescriptor{ 
        .minFilter = SamplerFilter::Nearest, 
        .magFilter = SamplerFilter::Nearest
    });

    if (!sample.sampler.isValid())
    {
        logError("Failed to create sample sampler");
        return false;
    }

    return true;
}