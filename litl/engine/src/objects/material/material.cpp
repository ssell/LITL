#include "litl-engine/objects/material/material.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    struct Material::Impl
    {
        Renderer const* renderer = nullptr;
        MaterialDescriptor descriptor;
        MaterialProperties properties;
        GraphicsPipelineHandle graphicsPipelineHandle{};
        ComputePipelineHandle computePipelineHandle{};

        bool create(MaterialDescriptor const& materialDescriptor, Renderer const& pRenderer, ObjectPool& objectPool) noexcept
        {
            descriptor = materialDescriptor;
            renderer = &pRenderer;

            // ---------------------------------------------------------------------------------
            // --- Shader Module Handles

            auto createShaderModuleHandle = [&](ShaderResourceDescriptor& descriptor) noexcept -> ShaderModuleHandle
                {
                    if (descriptor.resource.empty() || descriptor.bytes.empty())
                    {
                        return {};
                    }

                    return renderer->createShaderModule(ShaderModuleDescriptor{
                        .resource = descriptor.resource,
                        .bytes = descriptor.bytes
                    });
                };

            auto vertexHandle = createShaderModuleHandle(descriptor.vertexShader);
            auto fragmentHandle = createShaderModuleHandle(descriptor.fragmentShader);
            auto geometryHandle = createShaderModuleHandle(descriptor.geometryShader);
            auto tessellationControlHandle = createShaderModuleHandle(descriptor.tessellationControlShader);
            auto tessellationEvaluationHandle = createShaderModuleHandle(descriptor.tessellationControlShader);
            auto meshHandle = createShaderModuleHandle(descriptor.meshShader);
            auto taskHandle = createShaderModuleHandle(descriptor.taskShader);
            auto computeHandle = createShaderModuleHandle(descriptor.computeShader);

            // Clear the temporary span viewing the shader bytes
            descriptor.vertexShader.bytes = {};
            descriptor.fragmentShader.bytes = {};
            descriptor.geometryShader.bytes = {};
            descriptor.tessellationControlShader.bytes = {};
            descriptor.tessellationEvaluationShader.bytes = {};
            descriptor.meshShader.bytes = {};
            descriptor.taskShader.bytes = {};
            descriptor.computeShader.bytes = {};

            // ---------------------------------------------------------------------------------
            // --- Standard Graphics Pipeline (vertex + fragment + optional geometry/tessellation)

            if (vertexHandle.isValid() && fragmentHandle.isValid())
            {
                // --- Base Graphics Pipeline Descriptor

                GraphicsPipelineDescriptor graphicsPipelineDescriptor{
                    .vertex = PipelineShaderDescriptor {
                        .handle = vertexHandle,
                        .stage = ShaderStage::Vertex,
                        .entryPoint = descriptor.vertexShader.entryPoint
                    },
                    .fragment = PipelineShaderDescriptor {
                        .handle = fragmentHandle,
                        .stage = ShaderStage::Fragment,
                        .entryPoint = descriptor.fragmentShader.entryPoint
                    },
                    .geometry = PipelineShaderDescriptor {
                        .handle = geometryHandle,
                        .stage = ShaderStage::Geometry,
                        .entryPoint = descriptor.geometryShader.entryPoint
                    },
                    .tessellationControl = PipelineShaderDescriptor {
                        .handle = tessellationControlHandle,
                        .stage = ShaderStage::TessellationControl,
                        .entryPoint = descriptor.tessellationControlShader.entryPoint
                    },
                    .tessellationEvaluation = PipelineShaderDescriptor {
                        .handle = tessellationEvaluationHandle,
                        .stage = ShaderStage::TessellationEvaluation,
                        .entryPoint = descriptor.tessellationEvaluationShader.entryPoint
                    },
                    .renderTargets = RenderTargetFormats {                                      // ... todo expand this functionality ...
                        .colorAttachments = { renderer->getSwapchainImageFormat() },
                        .colorAttachmentCount = 1u,
                        .depthFormat = renderer->getSwapchainDepthFormat(),
                        .stencilFormat = renderer->getSwapchainDepthFormat()
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

                graphicsPipelineHandle = renderer->createGraphicsPipeline(graphicsPipelineDescriptor);

                // --- Configure Material Properties

                configurePropertiesFromReflection(vertexHandle);
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

        /// <summary>
        /// Attempts to retrieve the shader reflection data and extract the Push Constant .materialProperties field.
        /// With this field, we iterate its contents and populate the MaterialProperties.
        /// </summary>
        void configurePropertiesFromReflection(ShaderModuleHandle handle) noexcept
        {
            auto* reflection = renderer->getShaderReflection(handle);
            bool configuredProperties = false;

            if (reflection != nullptr)
            {
                auto entryPoint = reflection->getEntryPoint(descriptor.vertexShader.entryPoint);

                if (entryPoint.has_value() && (*entryPoint != nullptr))
                {
                    for (uint32_t i = 0u; (i < static_cast<uint32_t>((*entryPoint)->pushConstants.size())) && !configuredProperties; ++i)
                    {
                        auto& pushConstant = (*entryPoint)->pushConstants[i];

                        for (uint32_t j = 0u; (j < static_cast<uint32_t>(pushConstant.referenceProperties.size())) && !configuredProperties; ++j)
                        {
                            auto& reflectedStruct = pushConstant.referenceProperties[j];

                            if (reflectedStruct.hashedName == MaterialPropertiesStructHashedName)
                            {
                                properties.configure(MaterialPropertyReflection{
                                    .sizeBytes = reflectedStruct.stride,
                                    .properties = reflectedStruct.properties
                                    });

                                configuredProperties = true;
                            }
                        }
                    }

                    if (!configuredProperties)
                    {
                        logWarning("Failed to find material properties structure '", MaterialPropertiesStructName, "' in the Vertex Shader entry point '", descriptor.vertexShader.entryPoint, "' for material '", descriptor.objectInfo.name, "'");
                    }
                }
                else
                {
                    logWarning("Failed to retrieve Vertex Shader entry point '", descriptor.vertexShader.entryPoint, "' for material '", descriptor.objectInfo.name, "'");
                }
            }
            else
            {
                logWarning("Failed to retrieve Vertex Shader reflection data for material '", descriptor.objectInfo.name, "' from vertex shader '", descriptor.vertexShader.resource, "'");
            }
        }

        void destroy() noexcept
        {
            renderer->destroyGraphicsPipeline(graphicsPipelineHandle);
            renderer->destroyComputePipeline(computePipelineHandle);
        }
    };

    Material::Material()
        : m_pImpl(std::make_unique<Material::Impl>())
    {

    }

    Material::Material(Material&& other) = default;
    Material& Material::operator=(Material&& other) = default;

    Material::~Material()
    {

    }

    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept
    {
        return m_pImpl->create(descriptor, renderer, objectPool);
    }
    

    void Material::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_pImpl->destroy();
    }

    GraphicsPipelineHandle Material::getGraphicsPipelineHandle() const noexcept
    {
        return m_pImpl->graphicsPipelineHandle;
    }

    ComputePipelineHandle Material::getComputePipelineHandle() const noexcept
    {
        return m_pImpl->computePipelineHandle;
    }

    MaterialPropertySlotId Material::allocateSlot() noexcept
    {
        return m_pImpl->properties.allocateSlot();
    }
}