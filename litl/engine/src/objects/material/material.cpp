#include <format>
#include <optional>
#include <variant>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/string.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/assets/materialAsset.hpp"
#include "litl-engine/objects/material/material.hpp"
#include "litl-engine/objects/material/materialManager.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"
#include "litl-engine/objects/material/deferredMaterialCommands.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/shader.hpp"
#include "litl-engine/render/renderManager.hpp"
#include "litl-renderer/renderer.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl
{
    struct Material::Impl
    {
        struct MaterialVariant
        {
            MaterialVariantType type{};
            RenderAttachmentFormats formats{};
            GraphicsPipelineHandle handle{};
        };

        Renderer const* renderer = nullptr;
        ObjectPool* objectPool = nullptr;
        AssetManager* assetManager = nullptr;
        MaterialHandle selfHandle{};

        MaterialDescriptor descriptor;
        MaterialProperties properties;
        std::vector<MaterialVariant> variants;
        StringIdMap<uint32_t> specConstantMap;
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

        RenderAttachmentFormats renderSceneFormats{};
        uint32_t frameInFlightIndex{ 0u }; 

        // ---------------------------------------------------------------------------------
        // Creation Initiation
        // ---------------------------------------------------------------------------------

        bool create(MaterialDescriptor const& materialDescriptor, RenderManager const& renderManager, ObjectPool& inObjectPool) noexcept
        {
            descriptor = materialDescriptor;
            renderer = renderManager.getRenderer();
            objectPool = &inObjectPool;
            renderSceneFormats = renderManager.getSceneRenderFormats();

            if (!createInternalState(true, nullptr, {}))
            {
                return false;
            }

            properties.setReady();

            return true;
        }

        bool create(ObjectDescriptor const& objectDescriptor, RenderManager const& renderManager, ObjectPool& inObjectPool, AssetManager& inAssetManager) noexcept
        {
            descriptor.objectInfo = objectDescriptor;
            renderer = renderManager.getRenderer();
            objectPool = &inObjectPool;
            assetManager = &inAssetManager;
            renderSceneFormats = renderManager.getSceneRenderFormats();

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
            descriptor.blendState.mode = static_cast<MaterialBlendMode>(settings.blendMode);

            if (descriptor.blendState.mode == MaterialBlendMode::Transparent)
            {
                if (settings.blendPreset != import::LitlMatBlendPreset::None)
                {
                    descriptor.blendState.preset = static_cast<MaterialBlendPreset>(settings.blendPreset);

                    switch (descriptor.blendState.preset)
                    {
                    case MaterialBlendPreset::Alpha:
                        descriptor.blendState.colorBlend = ColorBlendState::alphaBlend();
                        break;

                    case MaterialBlendPreset::Premultiplied:
                        descriptor.blendState.colorBlend = ColorBlendState::premultiplied();
                        break;

                    case MaterialBlendPreset::Additive:
                        descriptor.blendState.colorBlend = ColorBlendState::additive();
                        break;

                    case MaterialBlendPreset::AdditiveAlpha:
                        descriptor.blendState.colorBlend = ColorBlendState::additiveAlphaWeighted();
                        break;

                    case MaterialBlendPreset::Multiply:
                        descriptor.blendState.colorBlend = ColorBlendState::multiply();
                        break;

                    case MaterialBlendPreset::Multiply2x:
                        descriptor.blendState.colorBlend = ColorBlendState::multiply2x();
                        break;

                    case MaterialBlendPreset::Screen:
                        descriptor.blendState.colorBlend = ColorBlendState::screen();
                        break;

                    case MaterialBlendPreset::Min:
                        descriptor.blendState.colorBlend = ColorBlendState::min();
                        break;

                    case MaterialBlendPreset::Max:
                        descriptor.blendState.colorBlend = ColorBlendState::max();
                        break;

                    default:
                        logWarning("Material '", descriptor.objectInfo.name, "' provided with unknown blend preset enum of ", static_cast<uint32_t>(descriptor.blendState.preset), ". Ignored.");
                        break;
                    }
                }
                else
                {
                    descriptor.blendState.colorBlend = ColorBlendState::create(
                        static_cast<BlendFactor>(settings.srcColor),
                        static_cast<BlendFactor>(settings.dstColor),
                        static_cast<BlendOperationType>(settings.colorBlendOp),
                        static_cast<BlendFactor>(settings.srcAlpha),
                        static_cast<BlendFactor>(settings.dstAlpha),
                        static_cast<BlendOperationType>(settings.alphaBlendOp)
                    );
                }
            }
            else
            {
                descriptor.blendState.colorBlend = ColorBlendState::noBlend();
            }

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

                case import::LitlMatPropertyType::Texture:
                    {
                        auto const* defaultValue = std::get_if<std::string>(&defaultProperty.value);

                        if (defaultValue != nullptr)
                        {
                            // Global texture index
                            auto* textureAsset = assetManager->getTexture(*defaultValue);

                            if (textureAsset != nullptr)
                            {
                                properties.setTexture(StringId(defaultProperty.name), textureAsset->texture, {}, true);
                            }
                            else
                            {
                                // Check if the user is requesting one of the reserved textures.
                                auto reservedIndex = getReservedTextureTableIndex(*defaultValue);

                                if (reservedIndex.has_value())
                                {
                                    properties.setTextureIndex(StringId(defaultProperty.name), static_cast<uint32_t>(reservedIndex.value()), {}, true);
                                }
                                else
                                {
                                    logWarning("Material '", descriptor.objectInfo.name, "' failed to retrieve default texture '", defaultValue->c_str(), "' for property '", defaultProperty.name, "'.");
                                }
                            }
                        }
                        else
                        {
                            logWarning("Material '", descriptor.objectInfo.name, "' specified a default value for non-global texture table texture property '", defaultProperty.name, "'. This property type is currently unsupported.");
                        }
                    }
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

            if (!createGraphicsInternals())
            {
                return false;
            }

            // ... todo mesh/compute ...

            return true;
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

        bool createGraphicsInternals() noexcept
        {
            // Specialization constants must be set prior to the graphics pipeline(s) being created.
            configureSpecConstants();

            // Initial graphics pipeline targetting the render scene formats.
            if (!createGraphicsPipelineVariants(renderSceneFormats))
            {
                logError("Material '", descriptor.objectInfo.name, "' failed create graphics pipeline(s).");
                return false;
            }

            // Reflect Properties
            if (!configureGraphicsPropertiesFromReflection())
            {
                logError("Material '", descriptor.objectInfo.name, "' failed to reflect properties.");
                return false;
            }

            // GPU Buffer
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

            return true;
        }

        [[nodiscard]] bool createGraphicsPipelineVariants(RenderAttachmentFormats const& attachmentFormats) noexcept
        {
            if (!createGraphicsPipeline(MaterialVariantType::Color, attachmentFormats).isValid())
            {
                logError("Material '", descriptor.objectInfo.name, "' failed to create color graphics pipeline variant.");
                return false;
            }

            if (!createGraphicsPipeline(MaterialVariantType::DepthOnly, attachmentFormats).isValid())
            {
                logError("Material '", descriptor.objectInfo.name, "' failed to create depth-only graphics pipeline variant.");
                return false;
            }

            return true;
        }

        GraphicsPipelineHandle createGraphicsPipeline(MaterialVariantType type, RenderAttachmentFormats const& attachmentFormats) noexcept
        {
            GraphicsPipelineHandle graphicsPipelineHandle = getGraphicsPipelineHandle(type, attachmentFormats);

            if (graphicsPipelineHandle.isValid())
            {
                return graphicsPipelineHandle;
            }

            if (vertexHandle.isValid() && fragmentHandle.isValid())
            {
                // --- Base Graphics Pipeline Descriptor

                GraphicsPipelineDescriptor graphicsPipelineDescriptor{};
                graphicsPipelineDescriptor.vertex = PipelineShaderDescriptor{ .handle = vertexHandle, .stage = ShaderStage::Vertex, .entryPoint = descriptor.vertexShader.entryPoint };
                
                if (type == MaterialVariantType::Color)
                {
                    graphicsPipelineDescriptor.fragment = PipelineShaderDescriptor{ .handle = fragmentHandle, .stage = ShaderStage::Fragment, .entryPoint = descriptor.fragmentShader.entryPoint }; 
                    graphicsPipelineDescriptor.geometry = PipelineShaderDescriptor{ .handle = geometryHandle, .stage = ShaderStage::Geometry, .entryPoint = descriptor.geometryShader.entryPoint };
                    graphicsPipelineDescriptor.tessellationControl = PipelineShaderDescriptor{ .handle = tessellationControlHandle, .stage = ShaderStage::TessellationControl, .entryPoint = descriptor.tessellationControlShader.entryPoint }; 
                    graphicsPipelineDescriptor.tessellationEvaluation = PipelineShaderDescriptor{ .handle = tessellationEvaluationHandle, .stage = ShaderStage::TessellationEvaluation, .entryPoint = descriptor.tessellationEvaluationShader.entryPoint };

                    graphicsPipelineDescriptor.renderTargets.colorAttachmentCount = 1u;
                    graphicsPipelineDescriptor.renderTargets.colorAttachments = { attachmentFormats.color };

                    if (descriptor.blendState.mode == MaterialBlendMode::Transparent)
                    {
                        graphicsPipelineDescriptor.colorBlend = descriptor.blendState.colorBlend;
                    }
                }
                else if (type == MaterialVariantType::DepthOnly)
                {
                    // DepthOnly variant is the vertex shader only.
                }

                graphicsPipelineDescriptor.renderTargets.depthFormat = attachmentFormats.depth;
                graphicsPipelineDescriptor.renderTargets.stencilFormat = dataFormatHasStencil(attachmentFormats.depth) ? attachmentFormats.depth : DataFormat::Undefined;

                graphicsPipelineDescriptor.vertexInput = VertexInputState{};                                          // ... todo expand this functionality ...
                graphicsPipelineDescriptor.inputAssembly = InputAssemblyState{};                                      // ... todo expand this functionality ...
                graphicsPipelineDescriptor.tessellation = std::nullopt;                                               // ... todo expand this functionality ...
                graphicsPipelineDescriptor.rasterization = descriptor.rasterizerState;
                graphicsPipelineDescriptor.multisample = MultisampleState{};                                          // ... todo expand this functionality ...
                graphicsPipelineDescriptor.depthStencil = DepthStencilState{};
                graphicsPipelineDescriptor.dynamicState = DynamicStateMask{};                                         // ... todo expand this functionality ...
                graphicsPipelineDescriptor.specializationConstants = descriptor.specializationConstants;

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

                if (!graphicsPipelineHandle.isValid())
                {
                    logWarning("Failed to create graphics pipeline for material '", descriptor.objectInfo.name, "'");
                    return {};
                }

                variants.emplace_back(type, attachmentFormats, graphicsPipelineHandle);
            }

            return graphicsPipelineHandle;
        }

        GraphicsPipelineHandle getGraphicsPipelineHandle(MaterialVariantType type, RenderAttachmentFormats const& attachmentFormats) const noexcept
        {
            for (auto& variant : variants)
            {
                if ((variant.type == type) && (variant.formats == attachmentFormats))
                {
                    return variant.handle;
                }
            }

            return GraphicsPipelineHandle{};
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

                if (!properties.configure(MaterialPropertyReflection{ .sizeBytes = propertyStructSizeBytes, .properties = std::move(reflectedProperties) }, framesInFlight, *objectPool))
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

        void configureSpecConstants() noexcept
        {
            // Reflect spec constants
            std::vector<SpecializationConstant> reflectedSpecConstants;

            compileReflectedSpecConstants(vertexHandle, reflectedSpecConstants);
            compileReflectedSpecConstants(fragmentHandle, reflectedSpecConstants);
            compileReflectedSpecConstants(geometryHandle, reflectedSpecConstants);
            compileReflectedSpecConstants(tessellationEvaluationHandle, reflectedSpecConstants);

            for (const auto& sc : reflectedSpecConstants)
            {
                specConstantMap[StringId(sc.name)] = sc.id;
            }

            // Set spec constants based on descriptor properties
            switch (descriptor.blendState.mode)
            {
            case MaterialBlendMode::Transparent:
                setConstantBool(InternalSpecializationConstantNames[static_cast<uint32_t>(InternalSpecializationConstant::RenderBlended)], true);
                break;

            case MaterialBlendMode::Masked:
                setConstantBool(InternalSpecializationConstantNames[static_cast<uint32_t>(InternalSpecializationConstant::RenderMasked)], true);
                break;

            case MaterialBlendMode::Opaque:
                setConstantBool(InternalSpecializationConstantNames[static_cast<uint32_t>(InternalSpecializationConstant::RenderOpaque)], true);
                break;

            default:
                logWarning("Unknown material blend mode ", static_cast<uint32_t>(descriptor.blendState.mode), " during spec constant configuration. Defaulting to opaque.");
                setConstantBool(InternalSpecializationConstantNames[static_cast<uint32_t>(InternalSpecializationConstant::RenderOpaque)], true);
                break;
            }

            if (descriptor.blendState.preset == MaterialBlendPreset::Premultiplied)
            {
                setConstantBool(InternalSpecializationConstantNames[static_cast<uint32_t>(InternalSpecializationConstant::RenderBlendedPremultiply)], true);
            }
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

            // Reflected material properties from the material struct
            for (uint32_t i = 0u; i < static_cast<uint32_t>((*entryPoint)->pushConstants.size()); ++i)
            {
                const auto& pushConstant = (*entryPoint)->pushConstants[i];

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

            return true;
        }

        void compileReflectedSpecConstants(ShaderModuleHandle handle, std::vector<SpecializationConstant>& reflectedSpecConstants) noexcept
        {
            if (!handle.isValid())
            {
                return;
            }

            auto* reflection = renderer->getShaderReflection(handle);

            if (reflection == nullptr)
            {
                logWarning("Failed to retrieve shader reflection data for material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
                return;
            }

            constexpr auto isSpecConstantDuplicated = [](SpecializationConstant const& specConstant, std::vector<SpecializationConstant>& known)
                {
                    for (const auto& sc : known)
                    {
                        if ((specConstant.id == sc.id) || (specConstant.name == sc.name))
                        {
                            return true;
                        }
                    }

                    return false;
                };

            // Reflected specialization constants
            for (uint32_t i = 0u; i < static_cast<uint32_t>(reflection->specializationConstants.size()); ++i)
            {
                const auto& specConstant = reflection->specializationConstants[i];

                if (isSpecConstantDuplicated(specConstant, reflectedSpecConstants))
                {
                    logWarning("Specialization constant duplicated on id (", specConstant.id, ") or name (", specConstant.name, "). Rejecting.");
                    continue;
                }

                reflectedSpecConstants.push_back(specConstant);
            }
        }

        // ---------------------------------------------------------------------------------
        // Set Specialization Constants
        // ---------------------------------------------------------------------------------
         
        // The following functions simply update the material descriptor accordingly. A change to spec constants requires
        // a rebuild of the underlying graphics pipeline. When users are allows to set spec constants it
        // needs to be done in a way where either they are specified before the initial graphics pipeline
        // is created or it triggers a rebuild of it.

        bool setConstantBool(StringId name, bool value) noexcept
        {
            const auto findSC = specConstantMap.find(name);

            if (findSC == specConstantMap.end())
            {
                return false;
            }

            descriptor.specializationConstants.addEntry(findSC->second, value);

            return true;
        }

        bool setConstantInt32(StringId name, int32_t value) noexcept
        {
            const auto findSC = specConstantMap.find(name);

            if (findSC == specConstantMap.end())
            {
                return false;
            }

            descriptor.specializationConstants.addEntry(findSC->second, value);

            return true;
        }

        bool setConstantUint32(StringId name, uint32_t value) noexcept
        {
            const auto findSC = specConstantMap.find(name);

            if (findSC == specConstantMap.end())
            {
                return false;
            }

            descriptor.specializationConstants.addEntry(findSC->second, value);

            return true;
        }

        bool setConstantFloat(StringId name, float value) noexcept
        {
            const auto findSC = specConstantMap.find(name);

            if (findSC == specConstantMap.end())
            {
                return false;
            }

            descriptor.specializationConstants.addEntry(findSC->second, value);

            return true;
        }

        bool setConstantDouble(StringId name, double value) noexcept
        {
            const auto findSC = specConstantMap.find(name);

            if (findSC == specConstantMap.end())
            {
                return false;
            }

            descriptor.specializationConstants.addEntry(findSC->second, value);

            return true;
        }

        // ---------------------------------------------------------------------------------
        // Tear Down
        // ---------------------------------------------------------------------------------

        void destroy() noexcept
        {
            if (renderer != nullptr)
            {
                for (auto& variant : variants)
                {
                    renderer->destroyGraphicsPipeline(variant.handle);
                }

                renderer->destroyComputePipeline(computePipelineHandle); 
                renderer->destroyShaderModule(vertexHandle); 
                renderer->destroyShaderModule(fragmentHandle); 
                renderer->destroyShaderModule(geometryHandle); 
                renderer->destroyShaderModule(tessellationControlHandle); 
                renderer->destroyShaderModule(tessellationEvaluationHandle); 
                renderer->destroyShaderModule(meshHandle); 
                renderer->destroyShaderModule(taskHandle); 
                renderer->destroyShaderModule(computeHandle); 

                variants.clear();
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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
            if (!slot.isValid() && !isDefault)
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

        bool setTexture(StringId property, TextureHandle handle, MaterialPropertySlotId slot, bool isDefault) noexcept
        {
            if (!slot.isValid() && !isDefault)
            {
                return false;
            }

            if (!properties.setTexture(property, objectPool->getTexture(handle), slot, isDefault))
            {
                logWarning("Failed to set ", (isDefault ? "default " : ""), "texture value in material '", descriptor.objectInfo.name, "'. Is there a mismatch between set call and/or material definition and/or shader implementation?");
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

    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, RenderManager const& renderManager, ObjectPool& objectPool) noexcept
    {
        return m_pImpl->create(descriptor, renderManager, objectPool);
    }

    bool Material::create(Authority<ObjectPool> auth, ObjectDescriptor const& descriptor, RenderManager const& renderManager, ObjectPool& objectPool, AssetManager& assetManager) noexcept
    {
        return m_pImpl->create(descriptor, renderManager, objectPool, assetManager);
    }
    
    void Material::setSelfHandle(Authority<ObjectPool> auth, MaterialHandle selfHandle) noexcept
    {
        LITL_ASSERT_MSG(selfHandle.isValid(), "Material provided with invalid self handle.", );

        m_pImpl->selfHandle = selfHandle;
        m_pImpl->properties.setMaterialHandle(selfHandle);
    }

    MaterialHandle Material::getHandle() const noexcept
    {
        return m_pImpl->selfHandle;
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

    GraphicsPipelineHandle Material::getGraphicsPipelineHandle(MaterialVariantType type, RenderAttachmentFormats const& attachmentFormats, bool createIfMissing) const noexcept
    {
        GraphicsPipelineHandle handle = m_pImpl->getGraphicsPipelineHandle(type, attachmentFormats);

        if (!handle.isValid() && createIfMissing)
        {
            handle = m_pImpl->createGraphicsPipeline(type, attachmentFormats);
        }

        return handle;
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

    MaterialBinding Material::allocateBinding() noexcept
    {
        return MaterialBinding{
            .handle = m_pImpl->selfHandle,
            .slot = allocateSlot()
        };
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

    void Material::markActive(MaterialPropertySlotId slot) noexcept
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

    bool Material::setTexture(StringId property, TextureHandle handle, MaterialPropertySlotId slot) noexcept
    {
        return m_pImpl->setTexture(property, handle, slot, false);
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

    bool Material::setDefaultTexture(StringId property, TextureHandle handle) noexcept
    {
        return m_pImpl->setTexture(property, handle, {}, true);
    }

    MaterialBlendMode Material::getBlendMode() const noexcept
    {
        return m_pImpl->descriptor.blendState.mode;
    }

    // -------------------------------------------------------------------------------------
    // Various static asserts needed to ensure no drift between Material definitions in litl-engine and litl-import
    // -------------------------------------------------------------------------------------

    // CullMode == LitlMatCullMode
    static_assert(static_cast<uint32_t>(CullMode::Back) == static_cast<uint32_t>(import::LitlMatCullMode::Back));
    static_assert(static_cast<uint32_t>(CullMode::Front) == static_cast<uint32_t>(import::LitlMatCullMode::Front));
    static_assert(static_cast<uint32_t>(CullMode::None) == static_cast<uint32_t>(import::LitlMatCullMode::None));
    static_assert(static_cast<uint32_t>(CullMode::Both) == static_cast<uint32_t>(import::LitlMatCullMode::Both));

    // MaterialBlendMode == LitlMatBlendMode
    static_assert(static_cast<uint32_t>(MaterialBlendMode::Opaque) == static_cast<uint32_t>(import::LitlMatBlendMode::Opaque));
    static_assert(static_cast<uint32_t>(MaterialBlendMode::Masked) == static_cast<uint32_t>(import::LitlMatBlendMode::Masked));
    static_assert(static_cast<uint32_t>(MaterialBlendMode::Transparent) == static_cast<uint32_t>(import::LitlMatBlendMode::Transparent));

    // BlendOperationType == LitlMatBlendOperation
    static_assert(static_cast<uint32_t>(BlendOperationType::Add) == static_cast<uint32_t>(import::LitlMatBlendOperation::Add));
    static_assert(static_cast<uint32_t>(BlendOperationType::Subtract) == static_cast<uint32_t>(import::LitlMatBlendOperation::Subtract));
    static_assert(static_cast<uint32_t>(BlendOperationType::ReverseSubtract) == static_cast<uint32_t>(import::LitlMatBlendOperation::ReverseSubtract));
    static_assert(static_cast<uint32_t>(BlendOperationType::Min) == static_cast<uint32_t>(import::LitlMatBlendOperation::Min));
    static_assert(static_cast<uint32_t>(BlendOperationType::Max) == static_cast<uint32_t>(import::LitlMatBlendOperation::Max));

    // BlendFactor == LitlMatBlendFactor
    static_assert(static_cast<uint32_t>(BlendFactor::Zero) == static_cast<uint32_t>(import::LitlMatBlendFactor::Zero));
    static_assert(static_cast<uint32_t>(BlendFactor::One) == static_cast<uint32_t>(import::LitlMatBlendFactor::One));
    static_assert(static_cast<uint32_t>(BlendFactor::SrcColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::SrcColor));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusSrcColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusSrcColor));
    static_assert(static_cast<uint32_t>(BlendFactor::DstColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::DstColor));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusDstColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusDstColor));
    static_assert(static_cast<uint32_t>(BlendFactor::SrcAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::SrcAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusSrcAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusSrcAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::DstAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::DstAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusDstAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusDstAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::ConstantColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::ConstantColor));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusConstantColor) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusConstantColor));
    static_assert(static_cast<uint32_t>(BlendFactor::ConstantAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::ConstantAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::OneMinusConstantAlpha) == static_cast<uint32_t>(import::LitlMatBlendFactor::OneMinusConstantAlpha));
    static_assert(static_cast<uint32_t>(BlendFactor::SrcAlphaSaturate) == static_cast<uint32_t>(import::LitlMatBlendFactor::SrcAlphaSaturate));

    // MaterialBlendPreset == LitlMatBlendPreset
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::None) == static_cast<uint32_t>(import::LitlMatBlendPreset::None));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Alpha) == static_cast<uint32_t>(import::LitlMatBlendPreset::Alpha));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Premultiplied) == static_cast<uint32_t>(import::LitlMatBlendPreset::Premultiplied));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Additive) == static_cast<uint32_t>(import::LitlMatBlendPreset::Additive));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::AdditiveAlpha) == static_cast<uint32_t>(import::LitlMatBlendPreset::AdditiveAlpha));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Multiply) == static_cast<uint32_t>(import::LitlMatBlendPreset::Multiply));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Multiply2x) == static_cast<uint32_t>(import::LitlMatBlendPreset::Multiply2x));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Screen) == static_cast<uint32_t>(import::LitlMatBlendPreset::Screen));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Min) == static_cast<uint32_t>(import::LitlMatBlendPreset::Min));
    static_assert(static_cast<uint32_t>(MaterialBlendPreset::Max) == static_cast<uint32_t>(import::LitlMatBlendPreset::Max));
}