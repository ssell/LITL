#include "litl-core/assert.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-engine/objects/material.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer) noexcept
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

        // Clear the temporary span viewing the shader bytes
        m_descriptor.vertexShader.bytes = {};
        m_descriptor.fragmentShader.bytes = {};
        m_descriptor.geometryShader.bytes = {};
        m_descriptor.tessellationControlShader.bytes = {};
        m_descriptor.tessellationEvaluationShader.bytes = {};

        // ---------------------------------------------------------------------------------
        // --- Standard Graphics Pipeline (vertex + fragment + optional geometry/tessellation)

        if (vertexHandle.isValid() && fragmentHandle.isValid())
        {
            GraphicsPipelineDescriptor graphicsPipelineDescriptor {
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
                    .colorAttachmentCount = 1u
                },
                .vertexInput = VertexInputState{},                                          // ... todo expand this functionality ...
                .inputAssembly = InputAssemblyState{},                                      // ... todo expand this functionality ...
                .tessellation = std::nullopt,                                               // ... todo expand this functionality ...
                .rasterization = RasterizationState{},                                      // ... todo expand this functionality ...
                .multisample = MultisampleState{},                                          // ... todo expand this functionality ...
                .depthStencil = DepthStencilState{},                                        // ... todo expand this functionality ...
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

            if (vertexHandle.isValid())
            {
                // ... todo expand this functionality to support alternative/custom vertex input layouts ...
                graphicsPipelineDescriptor.vertexInput.addBinding(VertexBinding{
                    .binding = 0u,
                    .stride = sizeof(Vertex),
                    .rate = VertexInputRate::PerVertex
                });

                uint32_t runningAttributeOffset = 0u;

                // position
                graphicsPipelineDescriptor.vertexInput.addAttribute<vec3>(VertexAttribute{
                    .location = 0u,
                    .binding = 0u,
                    .format = DataFormat::RGB32_SFloat
                }, runningAttributeOffset);

                // color
                graphicsPipelineDescriptor.vertexInput.addAttribute<vec3>(VertexAttribute{
                    .location = 1u,
                    .binding = 0u,
                    .format = DataFormat::RGB32_SFloat
                }, runningAttributeOffset);

                // uv
                graphicsPipelineDescriptor.vertexInput.addAttribute<vec3>(VertexAttribute{
                    .location = 2u,
                    .binding = 0u,
                    .format = DataFormat::RG32_SFloat
                }, runningAttributeOffset);
            }

            m_graphicsPipelineHandle = m_pRenderer->createGraphicsPipeline(graphicsPipelineDescriptor);
        }
        
        // ... todo alt graphics path (mesh/task) ...

        // ---------------------------------------------------------------------------------
        // --- Compute Pipeline

        // ... todo ...

        return true;
    }

    ShaderModuleHandle Material::createShaderModuleHandle(ShaderResourceDescriptor& descriptor) const noexcept
    {
        return m_pRenderer->createShaderModule(ShaderModuleDescriptor{
            .resource = descriptor.resource,
            .bytes = descriptor.bytes
        });
    }

    void Material::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_pRenderer->destroyGraphicsPipeline(m_graphicsPipelineHandle);
        m_pRenderer->destroyComputePipeline(m_computePipelineHandle);
    }

    GraphicsPipelineHandle Material::getGraphicsPipelineHandle() const noexcept
    {
        return m_graphicsPipelineHandle;
    }

    ComputePipelineHandle Material::getComputePipelineHandle() const noexcept
    {
        return m_computePipelineHandle;
    }
}