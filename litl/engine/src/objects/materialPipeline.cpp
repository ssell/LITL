#include "litl-engine/objects/materialPipeline.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    bool MaterialPipeline::create(Authority<ObjectPool> auth, MaterialPipelineDescriptor const& descriptor, Renderer const& renderer) noexcept
    {
        m_descriptor = descriptor;
        m_pRenderer = &renderer;

        // ---------------------------------------------------------------------------------
        // --- Shader Module Handles

        auto vertexHandle = createShaderModuleHandle(m_descriptor.vertexShader);
        auto fragmentHandle = createShaderModuleHandle(m_descriptor.fragmentShader);
        auto geometryHandle = createShaderModuleHandle(m_descriptor.geometryShader);
        auto tessellationControlHandle = createShaderModuleHandle(m_descriptor.tessellationControlShader);
        auto tessellationEvaluationHandle = createShaderModuleHandle(m_descriptor.tessellationControlShader);
        auto meshHandle = createShaderModuleHandle(m_descriptor.meshShader);
        auto taskHandle = createShaderModuleHandle(m_descriptor.taskShader);
        auto computeHandle = createShaderModuleHandle(m_descriptor.computeShader);

        // Clear the temporary span viewing the shader bytes
        m_descriptor.vertexShader.bytes = {};
        m_descriptor.fragmentShader.bytes = {};
        m_descriptor.geometryShader.bytes = {};
        m_descriptor.tessellationControlShader.bytes = {};
        m_descriptor.tessellationEvaluationShader.bytes = {};
        m_descriptor.meshShader.bytes = {};
        m_descriptor.taskShader.bytes = {};
        m_descriptor.computeShader.bytes = {};

        // ---------------------------------------------------------------------------------
        // --- Standard Graphics Pipeline (vertex + fragment + optional geometry/tessellation)

        if (vertexHandle.isValid() && fragmentHandle.isValid())
        {
            // --- Base Graphics Pipeline Descriptor

            GraphicsPipelineDescriptor graphicsPipelineDescriptor{
                .vertex = PipelineShaderDescriptor {
                    .handle = vertexHandle,
                    .stage = ShaderStage::Vertex,
                    .entryPoint = m_descriptor.vertexShader.entryPoint
                },
                .fragment = PipelineShaderDescriptor {
                    .handle = fragmentHandle,
                    .stage = ShaderStage::Fragment,
                    .entryPoint = m_descriptor.fragmentShader.entryPoint
                },
                .geometry = PipelineShaderDescriptor {
                    .handle = geometryHandle,
                    .stage = ShaderStage::Geometry,
                    .entryPoint = m_descriptor.geometryShader.entryPoint
                },
                .tessellationControl = PipelineShaderDescriptor {
                    .handle = tessellationControlHandle,
                    .stage = ShaderStage::TessellationControl,
                    .entryPoint = m_descriptor.tessellationControlShader.entryPoint
                },
                .tessellationEvaluation = PipelineShaderDescriptor {
                    .handle = tessellationEvaluationHandle,
                    .stage = ShaderStage::TessellationEvaluation,
                    .entryPoint = m_descriptor.tessellationEvaluationShader.entryPoint
                },
                .renderTargets = RenderTargetFormats {                                      // ... todo expand this functionality ...
                    .colorAttachments = { m_pRenderer->getSwapchainImageFormat() },
                    .colorAttachmentCount = 1u,
                    .depthFormat = m_pRenderer->getSwapchainDepthFormat(),
                    .stencilFormat = m_pRenderer->getSwapchainDepthFormat()
                },
                .vertexInput = VertexInputState{},                                          // ... todo expand this functionality ...
                .inputAssembly = InputAssemblyState{},                                      // ... todo expand this functionality ...
                .tessellation = std::nullopt,                                               // ... todo expand this functionality ...
                .rasterization = descriptor.rasterizerState,
                .multisample = MultisampleState{},                                          // ... todo expand this functionality ...
                .depthStencil = DepthStencilState{

                },
                .colorBlend = ColorBlendState{                                              // ... todo expand this functionality ...
                    .logicOpEnabled = false,
                    .colorAttachmentBlendStates = {
                        ColorBlendAttachmentState {
                            .attachmentBlendEnabled = false
                        }
                    }
                },
                .dynamicState = DynamicStateMask{},                                         // ... todo expand this functionality ...
                .specializationConstants = SpecializationConstants{}                        // ... todo expand this functionality ...
            };

            // --- Vertex Input

            graphicsPipelineDescriptor.vertexInput.addBinding(VertexBinding{
                .binding = 0u,
                .stride = descriptor.inputDescriptor.vertexSize,
                .rate = VertexInputRate::PerVertex
                });

            uint32_t runningAttributeOffset = 0u;

            for (uint32_t i = 0u; i < VertexInputDescriptor::MaxVertexAttributes; ++i)
            {
                DataFormat format = descriptor.inputDescriptor.attributes[i];

                if (format == DataFormat::Undefined)
                {
                    break;
                }

                graphicsPipelineDescriptor.vertexInput.addAttribute(VertexAttribute{
                    .location = i,
                    .binding = 0u,
                    .format = format
                    }, runningAttributeOffset);
            }

            // --- Create the Pipeline

            m_graphicsPipelineHandle = m_pRenderer->createGraphicsPipeline(graphicsPipelineDescriptor);
        }

        // ---------------------------------------------------------------------------------
        // --- Alternate Graphics Pipeline (mesh + task)

        else if (meshHandle.isValid() && taskHandle.isValid())
        {
            // ... todo ...
        }

        // ---------------------------------------------------------------------------------
        // --- Compute Pipeline

        if (computeHandle.isValid())
        {
            // ... todo ...
        }

        return true;
    }

    ShaderModuleHandle MaterialPipeline::createShaderModuleHandle(ShaderResourceDescriptor& descriptor) const noexcept
    {
        if (descriptor.resource.empty() || descriptor.bytes.empty())
        {
            return {};
        }

        return m_pRenderer->createShaderModule(ShaderModuleDescriptor{
            .resource = descriptor.resource,
            .bytes = descriptor.bytes
            });
    }

    void MaterialPipeline::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_pRenderer->destroyGraphicsPipeline(m_graphicsPipelineHandle);
        m_pRenderer->destroyComputePipeline(m_computePipelineHandle);
    }

    GraphicsPipelineHandle MaterialPipeline::getGraphicsPipelineHandle() const noexcept
    {
        return m_graphicsPipelineHandle;
    }

    ComputePipelineHandle MaterialPipeline::getComputePipelineHandle() const noexcept
    {
        return m_computePipelineHandle;
    }
}