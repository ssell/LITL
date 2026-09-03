#include <format>
#include <optional>
#include <variant>

#include "litl-core/assert.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/objects/material/material.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"
#include "litl-engine/objects/material/deferredMaterialCommands.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/shader.hpp"
#include "litl-engine/ecs/systems/activeMaterialSystem.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl
{
    struct Material::Impl
    {
        Renderer const* renderer = nullptr;
        ObjectPool* objectPool = nullptr;
        AssetManager* assetManager = nullptr;
        MaterialHandle selfHandle{};

        MaterialDescriptor descriptor;
        MaterialProperties properties;
        GraphicsPipelineHandle graphicsPipelineHandle{};
        ComputePipelineHandle computePipelineHandle{};
        GpuBufferHandle gpuBufferHandle{};
        std::optional<uint64_t> currGraphicsGpuBufferDeviceAddress{};

        ShaderModuleHandle vertexHandle{};
        ShaderModuleHandle fragmentHandle{};
        ShaderModuleHandle geometryHandle{};
        ShaderModuleHandle tessellationControlHandle{};
        ShaderModuleHandle tessellationEvaluationHandle{};
        ShaderModuleHandle meshHandle{};
        ShaderModuleHandle taskHandle{};
        ShaderModuleHandle computeHandle{};

        std::vector<MaterialPropertyBlockPointer> dirtyPropertyBlocks;

        uint32_t frameInFlightIndex{ 0u }; 

        // ---------------------------------------------------------------------------------
        // Creation Initiation
        // ---------------------------------------------------------------------------------

        bool create(MaterialDescriptor const& materialDescriptor, Renderer const& pRenderer, ObjectPool& pObjectPool) noexcept
        {
            descriptor = materialDescriptor;
            renderer = &pRenderer;
            objectPool = &pObjectPool;

            if (!createInternalState(true, nullptr, {}))
            {
                return false;
            }

            properties.setReady();

            return true;
        }

        bool create(ObjectDescriptor const& objectDescriptor, Renderer const& pRenderer, ObjectPool& pObjectPool, AssetManager& pAssetManager) noexcept
        {
            descriptor.objectInfo = objectDescriptor;
            renderer = &pRenderer;
            objectPool = &pObjectPool;
            assetManager = &pAssetManager;

            return true;
        }

        bool setData(import::MaterialIntermediateData const& intermediateData, std::span<MaterialAssetShaderDependency const> shaderDependencies) noexcept
        {
            if (assetManager == nullptr)
            {
                return false;
            }

            auto& settings = intermediateData.getSettings();

            descriptor.objectInfo.name = settings.materialName;
            descriptor.rasterizerState.cullMode = static_cast<CullMode>(settings.cullMode);
            descriptor.rasterizerState.frontFace = (settings.clockwise ? FrontFace::Clockwise : FrontFace::CounterClockwise);
            properties.toggleTier3DataSeparation(!settings.frequentUpdates);
            // ... update descriptor as more settings are exposed in the .litlmat format ...

            if (!createInternalState(false, &intermediateData, shaderDependencies))
            {
                return false;
            }

            auto& defaultProperties = intermediateData.getProperties();

            for (auto& defaultProperty : defaultProperties)
            {
                switch (defaultProperty.type)
                {
                case import::LitlMatPropertyType::Bool:
                    {
                        auto const* defaultValue = std::get_if<uint8_t>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setBool(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Integer:
                    {
                        auto const* defaultValue = std::get_if<int32_t>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setInt32(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::UnsignedInteger:
                    {
                        auto const* defaultValue = std::get_if<uint32_t>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setUint32(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Float:
                    {
                        auto const* defaultValue = std::get_if<float>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setFloat(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Double:
                    {
                        auto const* defaultValue = std::get_if<double>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setDouble(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Vec2:
                    {
                        auto const* defaultValue = std::get_if<vec2>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setVec2(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Vec3:
                    {
                        auto const* defaultValue = std::get_if<vec3>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setVec3(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Vec4:
                    {
                        auto const* defaultValue = std::get_if<vec4>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setVec4(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Color:
                    {
                        auto const* defaultValue = std::get_if<color>(&defaultProperty.value);
                        if (defaultValue != nullptr) { properties.setColor(StringId(defaultProperty.name), *defaultValue, {}, true); }
                    }
                    break;

                case import::LitlMatPropertyType::Texture2D:
                    logWarning("Material '", descriptor.objectInfo.name, "' specified a default value for Texture2D property '", defaultProperty.name, "'. This property type is currently unsupported.");
                    break;

                case import::LitlMatPropertyType::Texture3D:
                    logWarning("Material '", descriptor.objectInfo.name, "' specified a default value for Texture3D property '", defaultProperty.name, "'. This property type is currently unsupported.");
                    break;

                case import::LitlMatPropertyType::Unknown:
                default:
                    logWarning("Material '", descriptor.objectInfo.name, "' specified a default value for unsupported property type ", static_cast<uint32_t>(defaultProperty.type));
                    break;
                }
            }

            properties.setReady();

            return true;
        }

        // ---------------------------------------------------------------------------------
        // Creation Implementation
        // ---------------------------------------------------------------------------------

        bool createInternalState(bool shadersFromDescriptor, import::MaterialIntermediateData const* intermediateData, std::span<MaterialAssetShaderDependency const> shaderDependencies)
        {
            if (shadersFromDescriptor)
            {
                retrieveShaderHandlesFromDescriptor();
            }
            else
            {
                if ((intermediateData == nullptr) || !retrieveShaderHandlesFromDependencies(*intermediateData, shaderDependencies))
                {
                    return false;
                }
            }

            return createGraphicsPipeline();
        }

        void retrieveShaderHandlesFromDescriptor() noexcept
        {
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
        }

        bool retrieveShaderHandlesFromDependencies(import::MaterialIntermediateData const& intermediateData, std::span<MaterialAssetShaderDependency const> shaderDependencies) noexcept
        {
            auto shaderStageToArrayIndex = [&](ShaderStage stage) noexcept -> uint32_t
            {
                switch (stage)      // Needed as ShaderStage is a bitmask while import::LitlMatShaderStage is a standard enum.
                {
                    case ShaderStage::Vertex: return static_cast<uint32_t>(import::LitlMatShaderStage::Vertex) - 1u;
                    case ShaderStage::Fragment: return static_cast<uint32_t>(import::LitlMatShaderStage::Fragment) - 1u;
                    case ShaderStage::Geometry: return static_cast<uint32_t>(import::LitlMatShaderStage::Geometry) - 1u;
                    case ShaderStage::TessellationControl: return static_cast<uint32_t>(import::LitlMatShaderStage::TessellationControl) - 1u;
                    case ShaderStage::TessellationEvaluation: return static_cast<uint32_t>(import::LitlMatShaderStage::TessellationEvaluation) - 1u;
                    case ShaderStage::Compute: return static_cast<uint32_t>(import::LitlMatShaderStage::Compute) - 1u;
                    case ShaderStage::Mesh: return static_cast<uint32_t>(import::LitlMatShaderStage::Mesh) - 1u;
                    case ShaderStage::Task: return static_cast<uint32_t>(import::LitlMatShaderStage::Task) - 1u;
                    default: return 0u;
                }
            };

            auto processShaderStage = [&](ShaderStage stage, import::LitlMatShaderRecord const& intermediateShader, ShaderModuleHandle& targetHandle, ShaderResourceDescriptor& targetDescriptor) noexcept -> bool
            {
                if ((static_cast<ShaderStage>(intermediateShader.stage) == stage) && !intermediateShader.resource.empty() && !intermediateShader.entry.empty())
                {
                    targetDescriptor.resource = intermediateShader.resource;
                    targetDescriptor.entryPoint = intermediateShader.entry;

                    for (auto& shaderAssetDependency : shaderDependencies)
                    {
                        if (shaderAssetDependency.stage == stage)
                        {
                            auto* shaderAsset = assetManager->getShader(shaderAssetDependency.handle);

                            if ((shaderAsset == nullptr) || (shaderAsset->shader == nullptr) || (shaderAsset->status != AssetStatus::InMemory))
                            {
                                return false;
                            }

                            targetHandle = shaderAsset->shader->getShaderModuleHandle();
                        }
                    }
                }

                return true;
            };

            auto& intermediateShaders = intermediateData.getShaders();
            Shader* currShader = nullptr;

            if (!processShaderStage(ShaderStage::Vertex, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Vertex)], vertexHandle, descriptor.vertexShader))
            {
                logError("Failed to retrieve expected Vertex Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::Fragment, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Fragment)], fragmentHandle, descriptor.fragmentShader))
            {
                logError("Failed to retrieve expected Fragment Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::Geometry, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Geometry)], geometryHandle, descriptor.geometryShader))
            {
                logError("Failed to retrieve expected Geometry Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::TessellationControl, intermediateShaders[shaderStageToArrayIndex(ShaderStage::TessellationControl)], tessellationControlHandle, descriptor.tessellationControlShader))
            {
                logError("Failed to retrieve expected Tessellation Control Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::TessellationEvaluation, intermediateShaders[shaderStageToArrayIndex(ShaderStage::TessellationEvaluation)], tessellationEvaluationHandle, descriptor.tessellationEvaluationShader))
            {
                logError("Failed to retrieve expected Tessellation Evaluation Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::Mesh, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Mesh)], meshHandle, descriptor.meshShader))
            {
                logError("Failed to retrieve expected Mesh Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::Task, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Task)], taskHandle, descriptor.taskShader))
            {
                logError("Failed to retrieve expected Task Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            if (!processShaderStage(ShaderStage::Compute, intermediateShaders[shaderStageToArrayIndex(ShaderStage::Compute)], computeHandle, descriptor.computeShader))
            {
                logError("Failed to retrieve expected Compute Shader asset for Material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool createGraphicsPipeline() noexcept
        {
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

                if (properties.propertyCount() > 0u)
                {
                    gpuBufferHandle = objectPool->createGpuBuffer(GpuBufferDescriptor{
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

                // Our GPU Buffer is buffered based on frames-in-flight, so we need to inform the MaterialProperties on the frame count so that all N buffers can be updated on changes.
                const uint32_t framesInFlight = renderer->getFrameData().framesInFlight;

                if (!properties.configure(MaterialPropertyReflection{ .sizeBytes = propertyStructSizeBytes, .properties = std::move(reflectedProperties) }, framesInFlight))
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
                logWarning("Failed to retrieve shader reflection data for material '", descriptor.objectInfo.name, "' for entry point '", entryPointName, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            auto entryPoint = reflection->getEntryPoint(entryPointName);

            if (!entryPoint.has_value() || (*entryPoint == nullptr))
            {
                logWarning("Failed to retrieve shader entry point '", entryPointName, "' for material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
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

        // ---------------------------------------------------------------------------------
        // Tear Down
        // ---------------------------------------------------------------------------------

        void destroy() noexcept
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
            }

            if (objectPool != nullptr)
            {
                objectPool->destroyGpuBuffer(gpuBufferHandle);
                gpuBufferHandle = {};
            }
        }

        // ---------------------------------------------------------------------------------
        // Lifecycle Updates
        // ---------------------------------------------------------------------------------

        void onFrameStart(uint32_t frame, uint32_t frameIndex) noexcept
        {
            properties.setCurrentFrame(frame);
            frameInFlightIndex = frameIndex;
        }

        void onPreRender() noexcept
        {
            if (objectPool == nullptr)
            {
                return;
            }

            properties.freeSlots();
            properties.rebuildFrequentUpdateBlock();

            auto* gpuBuffer = objectPool->getGpuBuffer(gpuBufferHandle);

            if (gpuBuffer != nullptr)
            {
                if (gpuBuffer->resizeBytes(properties.totalMemoryRequirements(), 2u))
                {
                    properties.markAllBlocksDirty();
                }

                gpuBuffer->swapBuffers(frameInFlightIndex);
                properties.gatherDirtyBlocks(dirtyPropertyBlocks);

                if (!dirtyPropertyBlocks.empty())
                {
                    const auto slotSizeBytes = properties.individualSlotMemoryRequirements();

                    for (auto& dirtyBlock : dirtyPropertyBlocks)
                    {
                        gpuBuffer->recordChunkDataWrite(GpuBufferChunk{
                            .sourcePtr = dirtyBlock.sourcePtr,
                            .offset = dirtyBlock.blockOffset
                        });
                    }

                    properties.clearDirtyBlocks();
                }

                MaterialPropertyBlockPointer frequentUpdateBlockPointer{};

                if (properties.gatherFrequentUpdateBlockPointer(frequentUpdateBlockPointer))
                {
                    gpuBuffer->recordChunkDataWrite(GpuBufferChunk{
                        .sourcePtr = frequentUpdateBlockPointer.sourcePtr,
                        .offset = frequentUpdateBlockPointer.blockOffset
                    });
                }

                currGraphicsGpuBufferDeviceAddress = gpuBuffer->getBufferDeviceAddress();
            }
        }

        bool setBool(StringId property, bool value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setBool(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "bool value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setInt32(StringId property, int32_t value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setInt32(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "int32 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setUint32(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "uint32 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setFloat(StringId property, float value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setFloat(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "float value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setDouble(StringId property, double value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setDouble(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "double value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setVec2(StringId property, vec2 value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setVec2(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "vec2 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setVec3(StringId property, vec3 value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setVec3(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "vec3 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setVec4(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "vec4 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setColor(StringId property, color const& value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setColor(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "color value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setMat3(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "mat3 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
        }

        bool setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid())
            {
                return false;
            }

            if (!properties.setMat4(property, value, slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "mat4 value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return false;
            }

            return true;
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

    bool Material::create(Authority<ObjectPool> auth, ObjectDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool, AssetManager& assetManager) noexcept
    {
        return m_pImpl->create(descriptor, renderer, objectPool, assetManager);
    }
    
    void Material::setSelfHandle(Authority<ObjectPool> auth, MaterialHandle selfHandle) noexcept
    {
        LITL_ASSERT_MSG(selfHandle.isValid(), "Material provided with invalid self handle.", );

        m_pImpl->selfHandle = selfHandle;
        m_pImpl->properties.setMaterialHandle(selfHandle);
    }

    void Material::destroy(Authority<ObjectPool> auth) noexcept
    {
        m_pImpl->destroy();
    }

    bool Material::setData(Authority<MaterialAsset> auth, import::MaterialIntermediateData const& data, std::span<MaterialAssetShaderDependency const> shaderDependencies) noexcept
    {
        return m_pImpl->setData(data, shaderDependencies);
    }

    void Material::toggleFrequentDataUpdateSeparation(bool enabled) noexcept
    {
        m_pImpl->properties.toggleTier3DataSeparation(enabled);
    }

    GraphicsPipelineHandle Material::getGraphicsPipelineHandle() const noexcept
    {
        return m_pImpl->graphicsPipelineHandle;
    }

    ComputePipelineHandle Material::getComputePipelineHandle() const noexcept
    {
        return m_pImpl->computePipelineHandle;
    }

    GpuBufferHandle Material::getGraphicsGpuBufferHandle() const noexcept
    {
        return m_pImpl->gpuBufferHandle;
    }

    std::optional<uint64_t> Material::getGraphicsBufferDeviceAddress() const noexcept
    {
        return m_pImpl->currGraphicsGpuBufferDeviceAddress;
    }

    MaterialPropertySlotId Material::allocateSlot() noexcept
    {
        return m_pImpl->properties.allocateSlot();
    }

    uint32_t Material::getSlotIndex(MaterialPropertySlotId slotId) const noexcept
    {
        return m_pImpl->properties.getSlotIndex(slotId);
    }
    
    void Material::onFrameStart(Authority<MaterialManager> auth, uint32_t frame, uint32_t frameInFlightIndex) noexcept
    {
        m_pImpl->onFrameStart(frame, frameInFlightIndex);
    }

    void Material::onPreRender(Authority<MaterialManager> auth) noexcept
    {
        m_pImpl->onPreRender();
    }

    void Material::markActive(Authority<ActiveMaterialSystem> auth, MaterialPropertySlotId slot) noexcept
    {
        m_pImpl->properties.markSlotActive(slot);
    }

    void Material::upgradeSlotToFrequentBlock(Authority<DeferredMaterialCommands> auth, MaterialPropertySlotId slotId) noexcept
    {
        m_pImpl->properties.upgradeSlotToFrequentBlock(slotId);
    }

    bool Material::ready() const noexcept
    {
        if (m_pImpl->properties.propertyCount() > 0u)
        {
            return m_pImpl->currGraphicsGpuBufferDeviceAddress.has_value();
        }

        return true;
    }

    bool Material::setBool(StringId property, bool value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setBool(property, value, slot, false);
    }

    bool Material::setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setInt32(property, value, slot, false);
    }

    bool Material::setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setUint32(property, value, slot, false);
    }

    bool Material::setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setFloat(property, value, slot, false);
    }

    bool Material::setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setDouble(property, value, slot, false);
    }

    bool Material::setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setVec2(property, value, slot, false);
    }

    bool Material::setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setVec3(property, value, slot, false);
    }

    bool Material::setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setVec4(property, value, slot, false);
    }

    bool Material::setColor(StringId property, color const& value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setColor(property, value, slot, false);
    }

    bool Material::setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setMat3(property, value, slot, false);
    }

    bool Material::setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setMat4(property, value, slot, false);
    }

    bool Material::setDefaultBool(StringId property, bool value) noexcept
    {
        return m_pImpl->setBool(property, value, {}, true);
    }

    bool Material::setDefaultInt32(StringId property, int32_t value) noexcept
    {
        return m_pImpl->setInt32(property, value, {}, true);
    }

    bool Material::setDefaultUint32(StringId property, uint32_t value) noexcept
    {
        return m_pImpl->setUint32(property, value, {}, true);
    }

    bool Material::setDefaultFloat(StringId property, float value) noexcept
    {
        return m_pImpl->setFloat(property, value, {}, true);
    }

    bool Material::setDefaultDouble(StringId property, double value) noexcept
    {
        return m_pImpl->setDouble(property, value, {}, true);
    }

    bool Material::setDefaultVec2(StringId property, vec2 value) noexcept
    {
        return m_pImpl->setVec2(property, value, {}, true);
    }

    bool Material::setDefaultVec3(StringId property, vec3 value) noexcept
    {
        return m_pImpl->setVec3(property, value, {}, true);
    }

    bool Material::setDefaultVec4(StringId property, vec4 const& value) noexcept
    {
        return m_pImpl->setVec4(property, value, {}, true);
    }

    bool Material::setDefaultColor(StringId property, color const& value) noexcept
    {
        return m_pImpl->setColor(property, value, {}, true);
    }

    bool Material::setDefaultMat3(StringId property, mat3 const& value) noexcept
    {
        return m_pImpl->setMat3(property, value, {}, true);
    }

    bool Material::setDefaultMat4(StringId property, mat4 const& value) noexcept
    {
        return m_pImpl->setMat4(property, value, {}, true);
    }

    // -------------------------------------------------------------------------------------
    // Various static asserts needed to ensure no drift between Material definitions in litl-engine and litl-import
    // -------------------------------------------------------------------------------------

    static_assert(static_cast<uint32_t>(CullMode::Back) == static_cast<uint32_t>(import::LitlMatCullMode::Back));
    static_assert(static_cast<uint32_t>(CullMode::Front) == static_cast<uint32_t>(import::LitlMatCullMode::Front));
    static_assert(static_cast<uint32_t>(CullMode::None) == static_cast<uint32_t>(import::LitlMatCullMode::None));
    static_assert(static_cast<uint32_t>(CullMode::Both) == static_cast<uint32_t>(import::LitlMatCullMode::Both));
}