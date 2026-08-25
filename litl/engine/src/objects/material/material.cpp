#include <format>

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
        GpuBufferHandle gpuBufferHandle{};

        ShaderModuleHandle vertexHandle{};
        ShaderModuleHandle fragmentHandle{};
        ShaderModuleHandle geometryHandle{};
        ShaderModuleHandle tessellationControlHandle{};
        ShaderModuleHandle tessellationEvaluationHandle{};
        ShaderModuleHandle meshHandle{};
        ShaderModuleHandle taskHandle{};
        ShaderModuleHandle computeHandle{};

        bool create(MaterialDescriptor const& materialDescriptor, Renderer const& pRenderer, ObjectPool& objectPool) noexcept
        {
            descriptor = materialDescriptor;
            renderer = &pRenderer;

            // ---------------------------------------------------------------------------------
            // --- Shader Module Handles

            auto createShaderModuleHandle = [&](ShaderResourceDescriptor& shaderDescriptor) noexcept -> ShaderModuleHandle
                {
                    if (shaderDescriptor.resource.empty() || shaderDescriptor.bytes.empty())
                    {
                        return {};
                    }

                    return renderer->createShaderModule(ShaderModuleDescriptor{
                        .resource = shaderDescriptor.resource,
                        .bytes = shaderDescriptor.bytes
                    });
                };

            vertexHandle = createShaderModuleHandle(descriptor.vertexShader);
            fragmentHandle = createShaderModuleHandle(descriptor.fragmentShader);
            geometryHandle = createShaderModuleHandle(descriptor.geometryShader);
            tessellationControlHandle = createShaderModuleHandle(descriptor.tessellationControlShader);
            tessellationEvaluationHandle = createShaderModuleHandle(descriptor.tessellationEvaluationShader);
            meshHandle = createShaderModuleHandle(descriptor.meshShader);
            taskHandle = createShaderModuleHandle(descriptor.taskShader);
            computeHandle = createShaderModuleHandle(descriptor.computeShader);

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

                if (!configureGraphicsPropertiesFromReflection())
                {
                    logWarning("Material '", descriptor.objectInfo.name, "' failed to reflect properties.");
                    return false;
                }

                // --- Create the GPU Buffer

                gpuBufferHandle = objectPool.createGpuBuffer(GpuBufferDescriptor{
                    .objectInfo = ObjectDescriptor {
                        .name = std::format("Material Property Buffer ({})", descriptor.objectInfo.name),
                        .lifetime = descriptor.objectInfo.lifetime
                    },
                    .type = BufferTypeFlagBits::BufferDeviceAddress,
                    .memoryUsage = BufferMemoryUsage::PersistentMap,
                    .bufferStrategy = GpuBufferingStrategy::Frame,
                    .bytes = properties.totalMemoryRequirements() * 2,
                    .itemBytes = properties.individualSlotMemoryRequirements(),
                    .canResize = true
                });

                if (!gpuBufferHandle.isValid())
                {
                    logWarning("Material '", descriptor.objectInfo.name, "' failed to create properties GPU buffer.");
                    return false;
                }
            }

            // ---------------------------------------------------------------------------------
            // --- Alternate Graphics Pipeline (mesh + task)

            else if (meshHandle.isValid())      // task handle is optional
            {
                // ... todo ...
            }

            // ---------------------------------------------------------------------------------
            // --- Compute Pipeline

            if (computeHandle.isValid())
            {
                // ... todo ...
            }

            return (graphicsPipelineHandle.isValid() || computePipelineHandle.isValid());
        }

        bool configureGraphicsPropertiesFromReflection() noexcept
        {
            uint32_t propertyStructSizeBytes = 0u;
            std::vector<ResourceProperty> reflectedProperties;

            if (!compileReflectedProperties(vertexHandle, descriptor.vertexShader.entryPoint, propertyStructSizeBytes, reflectedProperties) ||
                !compileReflectedProperties(fragmentHandle, descriptor.fragmentShader.entryPoint, propertyStructSizeBytes, reflectedProperties) ||
                !compileReflectedProperties(geometryHandle, descriptor.geometryShader.entryPoint, propertyStructSizeBytes, reflectedProperties) ||
                !compileReflectedProperties(tessellationControlHandle, descriptor.tessellationControlShader.entryPoint, propertyStructSizeBytes, reflectedProperties) ||
                !compileReflectedProperties(tessellationEvaluationHandle, descriptor.tessellationEvaluationShader.entryPoint, propertyStructSizeBytes, reflectedProperties))
            {
                return false;
            }

            if (!reflectedProperties.empty())
            {
                if (propertyStructSizeBytes == 0u)
                {
                    logWarning("Material '", descriptor.objectInfo.name, "' reflection discovered properties, but struct has zero size. Rejecting.");
                    return false;
                }

                if (!properties.configure(MaterialPropertyReflection{ .sizeBytes = propertyStructSizeBytes, .properties = std::move(reflectedProperties) }))
                {
                    logWarning("Material '", descriptor.objectInfo.name, "' failed to populate properties. Rejecting.");
                    return false;
                }
            }
            else
            {
                if (propertyStructSizeBytes != 0u)
                {
                    logWarning("Material '", descriptor.objectInfo.name, "' reflection failed to discover properties for struct with non-zero size. Rejecting.");
                    return false;
                }
            }

            return true;
        }

        bool compileReflectedProperties(ShaderModuleHandle handle, std::string_view entryPointName, uint32_t& propertyStructSizeBytes, std::vector<ResourceProperty>& reflectedProperties) noexcept
        {
            if (!handle.isValid())
            {
                return true; // skip
            }

            auto* reflection = renderer->getShaderReflection(handle);

            if (reflection == nullptr)
            {
                logWarning("Failed to retrieve shader reflection data for material '", descriptor.objectInfo.name, "' for entry point '", entryPointName, "'");
                return false;
            }

            auto entryPoint = reflection->getEntryPoint(entryPointName);

            if (!entryPoint.has_value() || (*entryPoint == nullptr))
            {
                logWarning("Failed to retrieve shader entry point '", entryPointName, "' for material '", descriptor.objectInfo.name, "'");
                return false;
            }

            for (uint32_t i = 0u; i < static_cast<uint32_t>((*entryPoint)->pushConstants.size()); ++i)
            {
                auto& pushConstant = (*entryPoint)->pushConstants[i];

                for (uint32_t j = 0u; j < static_cast<uint32_t>(pushConstant.referenceProperties.size()); ++j)
                {
                    auto& reflectedStruct = pushConstant.referenceProperties[j];

                    if (reflectedStruct.hashedName == MaterialPropertiesStructHashedName)
                    {
                        if (propertyStructSizeBytes == 0u)
                        {
                            propertyStructSizeBytes = reflectedStruct.stride;
                        }
                        else if ((propertyStructSizeBytes != reflectedStruct.stride))
                        {
                            logWarning("Material properties reflected struct size mismatch detected. Rejecting.");
                            return false;
                        }

                        reflectedProperties.insert(reflectedProperties.end(), reflectedStruct.properties.begin(), reflectedStruct.properties.end());

                        return true;
                    }
                }
            }

            return true;    // No reflected properties is OK - the shader stage isn't using any.
        }

        void destroy(ObjectPool& objectPool) noexcept
        {
            if (renderer != nullptr)
            {
                renderer->destroyGraphicsPipeline(graphicsPipelineHandle); 
                renderer->destroyComputePipeline(computePipelineHandle); 
                renderer->destroyShaderModule(vertexHandle); 
                renderer->destroyShaderModule(fragmentHandle); 
                renderer->destroyShaderModule(geometryHandle); 
                renderer->destroyShaderModule(tessellationControlHandle); 
                renderer->destroyShaderModule(tessellationEvaluationHandle); 
                renderer->destroyShaderModule(meshHandle); 
                renderer->destroyShaderModule(taskHandle); 
                renderer->destroyShaderModule(computeHandle); 
                objectPool.destroyGpuBuffer(gpuBufferHandle);

                graphicsPipelineHandle = {};
                computePipelineHandle = {};
                vertexHandle = {};
                fragmentHandle = {};
                geometryHandle = {};
                tessellationControlHandle = {};
                tessellationEvaluationHandle = {};
                meshHandle = {};
                taskHandle = {};
                computeHandle = {};
                gpuBufferHandle = {};
            }
        }
    };

    Material::Material()
        : m_pImpl(std::make_unique<Material::Impl>())
    {

    }

    Material::Material(Material&& other) noexcept = default;
    Material& Material::operator=(Material&& other) noexcept = default;

    Material::~Material()
    {

    }

    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept
    {
        return m_pImpl->create(descriptor, renderer, objectPool);
    }
    

    void Material::destroy(Authority<ObjectPool> auth, ObjectPool& objectPool) noexcept
    {
        m_pImpl->destroy(objectPool);
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