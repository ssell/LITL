#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <spirv_reflect.h>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"
#include "litl-renderer/reflection.hpp"

/**
 * Reflection is done using the SPIRV-Reflect library to reflect from the raw SPIR-V bytecode.
 * The results of said reflection are then transposed into our common litl::ShaderReflection data structure.
 */

namespace
{
    struct SpirvTypeInfo
    {
        /// <summary>
        /// SPIR-V result <id> of the type.
        /// </summary>
        uint32_t typeId = 0u;

        /// <summary>
        /// Byte distance between consecutive elements. 0 = no ArrayStride decoration.
        /// </summary>
        uint32_t arrayStride = 0u;

        /// <summary>
        /// For OpTypePointer, the <id> of the pointee. 0 = not a pointer type.
        /// </summary>
        uint32_t pointeeTypeId = 0u;
    };

    /// <summary>
    /// Used for SPIR-V ArrayStride recovery.
    /// 
    /// SPIRV-Reflect does not surface the ArrayStride decoration for PhysicalStorageBuffer pointer types (traits.array.stride reports 0) so it is read from the module directly.
    /// 
    /// The stride is authoritative: it is the exact value the shader uses for `ptr[i]` address arithmetic, so it cannot disagree with what the GPU does. Computing it CPU-side would mean
    /// reimplementing whichever layout rules the producer applied (Slang names its BDA pointee types `*_natural`, which is neither std140 nor std430).
    /// </summary>
    struct SpirvTypeTable
    {
        /// <summary>
        /// Retrieves the SpirvTypeInfo with the given id. Returns null if no match found.
        /// </summary>
        [[nodiscard]] SpirvTypeInfo const* find(uint32_t typeId) const noexcept
        {
            for (auto const& info : types)
            {
                if (info.typeId == typeId)
                {
                    return &info;
                }
            }

            return nullptr;
        }

        /// <summary>
        /// Returns the stride for the type, or 0 if the module does not declare one.
        /// </summary>
        [[nodiscard]] uint32_t getArrayStride(uint32_t typeId) const noexcept
        {
            auto const* info = find(typeId);

            if (info == nullptr)
            {
                return 0u;
            }

            if (info->arrayStride != 0u)
            {
                return info->arrayStride;
            }

            // In the event the pointee was decorated with the array stride instead of the pointer ...
            if (info->pointeeTypeId != 0u)
            {
                auto const* pointee = find(info->pointeeTypeId);

                if (pointee != nullptr)
                {
                    return pointee->arrayStride;
                }
            }

            return 0u;
        }

        [[nodiscard]] static bool buildSpirvTypeTable(std::span<std::byte const> spirvBytes, SpirvTypeTable& table) noexcept
        {
            table.types.clear();

            // Opcode / decoration / storage-class constants come from the SPIRV-Headers that SPIRV-Reflect already vendors; spirv_reflect.h pulls in spirv.h transitively.
            static constexpr size_t HeaderWordCount = 5u;   // magic, version, generator, bound, schema

            if ((spirvBytes.size_bytes() % sizeof(uint32_t)) != 0u)
            {
                litl::logError("SPIR-V byte code length is not a multiple of 4.");
                return false;
            }

            const size_t moduleWordCount = spirvBytes.size_bytes() / sizeof(uint32_t);

            if (moduleWordCount < HeaderWordCount)
            {
                litl::logError("SPIR-V byte code is shorter than the 5-word header.");
                return false;
            }

            // The span carries no alignment guarantee, so words are read through memcpy rather than a reinterpret_cast. Clang folds this to a plain load.
            auto readWord = [spirvBytes](size_t index) noexcept -> uint32_t 
            {
                uint32_t value = 0u;
                std::memcpy(&value, spirvBytes.data() + (index * sizeof(uint32_t)), sizeof(uint32_t));
                return value;
            };

            if (readWord(0u) != SpvMagicNumber)
            {
                // 0x03022307 would indicate a module of opposing endianness, which is not supported.
                litl::logError("SPIR-V magic number mismatch.");
                return false;
            }

            table.types.reserve(32u);
            size_t wordIndex = HeaderWordCount;

            while (wordIndex < moduleWordCount)
            {
                const uint32_t instruction = readWord(wordIndex);
                const uint16_t opCode = static_cast<uint16_t>(instruction & 0xFFFFu);
                const uint16_t instructionWordCount = static_cast<uint16_t>(instruction >> 16u);

                // A zero word count would loop forever; a run-off means the tail is truncated. Everything gathered before this point is still valid, so keep it.
                if ((instructionWordCount == 0u) || ((wordIndex + instructionWordCount) > moduleWordCount))
                {
                    litl::logWarning("SPIR-V instruction stream is malformed at word ", wordIndex);
                    break;
                }

                switch (opCode)
                {
                case SpvOpDecorate:
                    // OpDecorate <target-id> <decoration> [literals...]
                    if ((instructionWordCount >= 4u) && (readWord(wordIndex + 2u) == SpvDecorationArrayStride))
                    {
                        spirvTypeEntry(table, readWord(wordIndex + 1u)).arrayStride = readWord(wordIndex + 3u);

                        /**
                         * TODO handle RowMajor (SpvDecorationRowMajor) and make a shader variable flag.
                         * Then in MaterialProperties::setMat3/setMat4 we would need to swizzle.
                         */
                    }
                    break;

                case SpvOpTypePointer:
                    // OpTypePointer <result-id> <storage-class> <pointee-type-id>
                    // Only PhysicalStorageBuffer pointers are recorded as capturing every pointer type would bloat the table (and the linear scan) for no benefit.
                    // OpTypeForwardPointer is deliberately ignored: it declares the <id> without a pointee, and the matching OpTypePointer always appears later in the module.
                    if ((instructionWordCount >= 4u) && (readWord(wordIndex + 2u) == SpvStorageClassPhysicalStorageBuffer))
                    {
                        spirvTypeEntry(table, readWord(wordIndex + 1u)).pointeeTypeId = readWord(wordIndex + 3u);
                    }
                    break;

                default:
                    break;
                }

                wordIndex += instructionWordCount;
            }

            return true;
        }

        std::vector<SpirvTypeInfo> types;

    private:

        /// <summary>
        /// Returns the entry for `typeId`, creating it if absent. 
        /// Note: the reference is invalidated by the next call, so use it immediately.
        /// </summary>
        [[nodiscard]] static SpirvTypeInfo& spirvTypeEntry(SpirvTypeTable& table, uint32_t typeId) noexcept
        {
            for (auto& info : table.types)
            {
                if (info.typeId == typeId)
                {
                    return info;
                }
            }

            table.types.push_back(SpirvTypeInfo{ .typeId = typeId });

            return table.types.back();
        }
    };
}

namespace litl
{
    ShaderResourceType fromSpvReflectResourceType(SpvReflectDescriptorType descriptorType);
    [[nodiscard]] ShaderVariable shaderVariableFromSpvReflectTypeDescription(SpvReflectTypeDescription* type) noexcept;
    [[nodiscard]] ResourceProperty resourcePropertyFromSpvReflectBlockVariable(SpvReflectBlockVariable const& block) noexcept;

    SpvReflectEntryPoint* selectEntryPoint(const char* entryPoint, SpvReflectShaderModule const* reflectedModule);
    bool reflectShaderStage(EntryPointReflection& litlReflection, SpvReflectEntryPoint const* entryPoint);
    bool reflectResourceBindings(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);
    bool reflectPushConstants(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint, SpirvTypeTable const& spirvTypeTable);
    bool reflectVertexInputs(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);
    bool reflectFragmentOutputs(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);
    bool reflectSpecializationConstants(ShaderReflection& litlReflection, SpvReflectShaderModule const* reflectedModule);
    bool reflectComputeInfo(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);

    std::optional<ShaderReflection> reflectSPIRV(std::span<std::byte const> spirvByteCode)
    {
        SpvReflectShaderModule reflectedSpvModule{};
        const SpvReflectResult reflectionResult = spvReflectCreateShaderModule(spirvByteCode.size_bytes(), spirvByteCode.data(), &reflectedSpvModule);

        if (reflectionResult != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed with result ", reflectionResult);
            return std::nullopt;
        }

        SpirvTypeTable spirvTypeTable{};

        if (!SpirvTypeTable::buildSpirvTypeTable(spirvByteCode, spirvTypeTable))
        {
            logError("SPIR-V type table parse failed.");
            return std::nullopt;
        }

        ShaderReflection shaderReflection{};
        shaderReflection.entryPoints.reserve(reflectedSpvModule.entry_point_count);

        for (uint32_t i = 0u; i < reflectedSpvModule.entry_point_count; ++i)
        {
            auto* reflectedSpvEntryPoint = &reflectedSpvModule.entry_points[i];

            EntryPointReflection entryPointReflection{};
            entryPointReflection.entryPoint = reflectedSpvEntryPoint->name;

            if (reflectShaderStage(entryPointReflection, reflectedSpvEntryPoint) &&
                reflectResourceBindings(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectPushConstants(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint, spirvTypeTable) &&
                reflectVertexInputs(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectFragmentOutputs(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectComputeInfo(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint))
            {
                shaderReflection.entryPoints.push_back(std::move(entryPointReflection));
            }
            else
            {
                logError("SPIR-V reflection failed to find desired entry point ", entryPointReflection.entryPoint);
            }
        }

        reflectSpecializationConstants(shaderReflection, &reflectedSpvModule);
        spvReflectDestroyShaderModule(&reflectedSpvModule);

        if (shaderReflection.entryPoints.size() == 0)
        {
            logWarning("SPIR-V reflection of shader resulted in 0 entry points.");
            return std::nullopt;
        }

        return shaderReflection;
    }

    SpvReflectEntryPoint* selectEntryPoint(const char* entryPoint, SpvReflectShaderModule const* reflectedModule)
    {
        for (uint32_t i = 0; i < reflectedModule->entry_point_count; ++i)
        {
            if (strcmp(entryPoint, reflectedModule->entry_points[i].name) == 0)
            {
                return &reflectedModule->entry_points[i];
            }
        }

        return nullptr;
    }

    bool reflectShaderStage(EntryPointReflection& litlReflection, SpvReflectEntryPoint const* entryPoint)
    {
        switch (entryPoint->shader_stage)
        {
        case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
            litlReflection.stage = ShaderStage::Vertex;
            break;

        case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
            litlReflection.stage = ShaderStage::Fragment;
            break;

        case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
            litlReflection.stage = ShaderStage::Geometry;
            break;

        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            litlReflection.stage = ShaderStage::TessellationControl;
            break;

        case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            litlReflection.stage = ShaderStage::TessellationEvaluation;
            break;

        case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
            litlReflection.stage = ShaderStage::Compute;
            break;

        case SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV:
            litlReflection.stage = ShaderStage::Mesh;
            break;

        case SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV:
            litlReflection.stage = ShaderStage::Task;
            break;

        default:
            litlReflection.stage = ShaderStage::None;
            logError("SPIR-V reflection of unsupported shader stage ", entryPoint->shader_stage);
            return false;
        }

        return true;
    }

    bool reflectResourceBindings(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        uint32_t resourceBindingsCount = 0u;
        auto result = spvReflectEnumerateEntryPointDescriptorBindings(reflectedModule, entryPoint->name, &resourceBindingsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate resource binding count with result ", result);
            return false;
        }

        if (resourceBindingsCount == 0u)
        {
            return true;
        }

        // we piggy-back off of vector here to ensure the memory storing the pointers is freed at the end of scope (failure or success)
        std::vector<SpvReflectDescriptorBinding*> resourceBindings(resourceBindingsCount);
        result = spvReflectEnumerateEntryPointDescriptorBindings(reflectedModule, entryPoint->name, &resourceBindingsCount, resourceBindings.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate resource bindings with result ", result);
            return false;
        }

        litlReflection.resources.reserve(resourceBindingsCount);

        for (uint32_t i = 0; i < resourceBindingsCount; ++i)
        {
            auto const& binding = *resourceBindings[i];

            if (binding.set >= 32u)
            {
                // Set indices >= 32 are invalid due to the DescriptorSetChangeTracker.
                // If we need to support more than 32 sets, then that is where the change will need to be done.
                logError("Shader reflection processing encountered binding set index = ", binding.set, ". Only set indices [0, 31] are valid.");
                return false;
            }

            ResourceBinding newResourceBinding{
                .type = fromSpvReflectResourceType(binding.descriptor_type),
                .set = binding.set,
                .binding = binding.binding,
                .arraySize = binding.count,
                .sizeBytes = binding.block.size,
                .name = (binding.name != nullptr ? binding.name : "")
            };

            if (!newResourceBinding.name.empty())
            {
                newResourceBinding.hashedName = StringId(newResourceBinding.name);
            }

            litlReflection.resources.push_back(newResourceBinding);

            auto& resourceBinding = litlReflection.resources.back();

            if (binding.block.member_count > 0)
            {
                for (uint32_t j = 0u; j < binding.block.member_count; ++j)
                {
                    resourceBinding.properties.push_back(resourcePropertyFromSpvReflectBlockVariable(binding.block.members[j]));
                }
            }
        }

        return true;
    }

    bool reflectPushConstants(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint, SpirvTypeTable const& spirvTypeTable)
    {
        uint32_t pushConstantBlocksCount = 0u;
        auto result = spvReflectEnumerateEntryPointPushConstantBlocks(reflectedModule, entryPoint->name, &pushConstantBlocksCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate push constants block count with result ", result);
            return false;
        }

        if (pushConstantBlocksCount == 0u)
        {
            return true;
        }

        std::vector<SpvReflectBlockVariable*> pushConstantBlocks(pushConstantBlocksCount);
        result = spvReflectEnumerateEntryPointPushConstantBlocks(reflectedModule, entryPoint->name, &pushConstantBlocksCount, pushConstantBlocks.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate push constant blocks with result ", result);
            return false;
        }

        litlReflection.pushConstants.reserve(pushConstantBlocksCount);

        for (uint32_t i = 0; i < pushConstantBlocksCount; ++i)
        {
            auto pushConstantBlock = *pushConstantBlocks[i];

            litlReflection.pushConstants.push_back(PushConstantRange{
                .offset = pushConstantBlock.offset,
                .sizeBytes = pushConstantBlock.size
            });

            auto& pushConstant = litlReflection.pushConstants.back();

            if (pushConstantBlock.member_count > 0)
            {
                pushConstant.properties.reserve(pushConstantBlock.member_count);
                pushConstant.referenceProperties.reserve(pushConstantBlock.member_count);

                for (uint32_t j = 0u; j < pushConstantBlock.member_count; ++j)
                {
                    auto const& pushConstantMember = pushConstantBlock.members[j];
                    auto pushConstantResource = resourcePropertyFromSpvReflectBlockVariable(pushConstantMember);

                    if (has_any(pushConstantResource.variable.flag, ShaderVariableFlagBits::Ref))
                    {
                        // Get the stride/array element size
                        const uint32_t stride = (pushConstantMember.type_description != nullptr) ? spirvTypeTable.getArrayStride(pushConstantMember.type_description->id) : 0u;

                        PushConstantReferenceProperty refProperty{
                            .offset = pushConstantResource.offset,
                            .sizeBytes = pushConstantResource.sizePadded,
                            .stride = stride,
                            .hashedName = pushConstantResource.hashedName,
                            .name = pushConstantResource.name
                        };

                        refProperty.properties.reserve(pushConstantMember.member_count);

                        for (uint32_t k = 0u; k < pushConstantMember.member_count; ++k)
                        {
                            refProperty.properties.push_back(resourcePropertyFromSpvReflectBlockVariable(pushConstantMember.members[k]));
                        }

                        // Sanity checks
                        uint32_t refPropertiesEnd = 0u;

                        for (auto const& property : refProperty.properties)
                        {
                            refPropertiesEnd = litl::max((property.offset + property.size), refPropertiesEnd);
                        }

                        if ((stride == 0u) && !refProperty.properties.empty())
                        {
                            logWarning("Reflected SPIR-V has reference properties but no calculated stride.");
                        }

                        LITL_ASSERT_MSG(((stride == 0u) || (stride >= refPropertiesEnd)), "Reflected SPIR-V ArrayStride is smaller than the property block that it describes.", false);

                        pushConstant.referenceProperties.push_back(std::move(refProperty));
                    }

                    pushConstant.properties.push_back(std::move(pushConstantResource));
                }
            }
        }

        return true;
    }

    bool reflectVertexInputs(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        if (litlReflection.stage != ShaderStage::Vertex)
        {
            return true;
        }

        uint32_t vertexInputsCount = 0;
        auto result = spvReflectEnumerateEntryPointInputVariables(reflectedModule, entryPoint->name, &vertexInputsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate input variable count with result ", result);
            return false;
        }

        std::vector<SpvReflectInterfaceVariable*> inputVariables(vertexInputsCount);
        result = spvReflectEnumerateEntryPointInputVariables(reflectedModule, entryPoint->name, &vertexInputsCount, inputVariables.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate input variables with result ", result);
            return false;
        }

        litlReflection.vertexInputs.reserve(vertexInputsCount);

        for (uint32_t i = 0; i < vertexInputsCount; ++i)
        {
            auto const& inputVariable = *inputVariables[i];

            if (inputVariable.built_in != -1)
            {
                // Skip built-in attributes (SV_Position, etc.)
                continue;
            }

            ShaderInputOutputVariable inoutVariable{
                .location = inputVariable.location,
                .variable = shaderVariableFromSpvReflectTypeDescription(inputVariable.type_description),
                .name = (inputVariable.name != nullptr ? inputVariable.name : "")
            };

            litlReflection.vertexInputs.push_back(inoutVariable);
        }

        return true;
    }

    bool reflectFragmentOutputs(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        if (litlReflection.stage != ShaderStage::Fragment)
        {
            return true;
        }

        uint32_t fragmentOutputsCount = 0;
        auto result = spvReflectEnumerateEntryPointOutputVariables(reflectedModule, entryPoint->name, &fragmentOutputsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate fragment output variable count with result ", result);
            return false;
        }

        std::vector<SpvReflectInterfaceVariable*> outputVariables(fragmentOutputsCount);
        result = spvReflectEnumerateEntryPointOutputVariables(reflectedModule, entryPoint->name, &fragmentOutputsCount, outputVariables.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate fragment output variables with result ", result);
            return false;
        }

        litlReflection.fragmentOutputs.reserve(fragmentOutputsCount);

        for (uint32_t i = 0; i < fragmentOutputsCount; ++i)
        {
            auto outputVariable = *outputVariables[i];

            ShaderInputOutputVariable inoutVariable{
                .location = outputVariable.location,
                .variable = shaderVariableFromSpvReflectTypeDescription(outputVariable.type_description),
                .name = (outputVariable.name != nullptr ? outputVariable.name : "")
            };

            litlReflection.fragmentOutputs.push_back(inoutVariable);
        }

        return true;
    }

    bool reflectSpecializationConstants(ShaderReflection& litlReflection, SpvReflectShaderModule const* reflectedModule)
    {
        uint32_t constantsCount = 0;
        auto result = spvReflectEnumerateSpecializationConstants(reflectedModule, &constantsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate specialization cosntants count with result ", result);
            return false;
        }

        std::vector<SpvReflectSpecializationConstant*> constants(constantsCount);
        result = spvReflectEnumerateSpecializationConstants(reflectedModule, &constantsCount, constants.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIR-V reflection failed to enumerate specialization constants with result ", result);
            return false;
        }

        litlReflection.specializationConstants.reserve(constantsCount);

        for (uint32_t i = 0; i < constantsCount; ++i)
        {
            auto constant = *constants[i];

            SpecializationConstant specConstant{ 
                .id = constant.constant_id,
                .variable = shaderVariableFromSpvReflectTypeDescription(constant.type_description),
                .name = (constant.name != nullptr ? constant.name : "") 
            };

            litlReflection.specializationConstants.push_back(specConstant);
        }

        return true;
    }

    bool reflectComputeInfo(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        if (litlReflection.stage == ShaderStage::Compute)
        {
            litlReflection.computeInfo = ComputeInfo{
                .localSizeX = entryPoint->local_size.x,
                .localSizeY = entryPoint->local_size.y,
                .localSizeZ = entryPoint->local_size.z
            };
        }

        return true;
    }

    ShaderResourceType fromSpvReflectResourceType(SpvReflectDescriptorType descriptorType)
    {
        switch (descriptorType)
        {
        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            return ShaderResourceType::Sampler;

        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
            return ShaderResourceType::UniformBuffer;

        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
            return ShaderResourceType::StorageBuffer;

        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return ShaderResourceType::ImageBuffer;

        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            return ShaderResourceType::SampledImage;

        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            return ShaderResourceType::StorageImage;

        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return ShaderResourceType::InputAttachment;

        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            return ShaderResourceType::AccelerationStructure;

        default:
            return ShaderResourceType::Unknown;
        }
    }

    ShaderVariable shaderVariableFromSpvReflectTypeDescription(SpvReflectTypeDescription* type) noexcept
    {
        ShaderVariable variable{};

        if (type == nullptr)
        {
            return variable;
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
        {
            variable.scalarType = ShaderScalarType::Bool;
            variable.componentCount = 1u;
            variable.flag |= ShaderVariableFlagBits::Bool;
            variable.scalarSize = (type->traits.numeric.scalar.width == 0u ? 32u : type->traits.numeric.scalar.width);
        }
        else if (type->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        {
            variable.scalarType = ShaderScalarType::Integer;
            variable.componentCount = 1u;
            variable.flag |= ShaderVariableFlagBits::Int;
            variable.scalarSize = type->traits.numeric.scalar.width;

            if (type->traits.numeric.scalar.signedness == 0u)
            {
                variable.flag |= ShaderVariableFlagBits::Unsigned;
            }

        }
        else if (type->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
        {
            variable.scalarType = ShaderScalarType::Float;
            variable.componentCount = 1u;
            variable.flag |= ShaderVariableFlagBits::Float;
            variable.scalarSize = type->traits.numeric.scalar.width;
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR)
        {
            variable.flag |= ShaderVariableFlagBits::Vector;
            variable.componentCount = type->traits.numeric.vector.component_count;
        }
        
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_MATRIX)
        {
            variable.flag |= ShaderVariableFlagBits::Matrix;
            variable.componentCount = type->traits.numeric.matrix.column_count * type->traits.numeric.matrix.row_count;
            variable.matrixStride = type->traits.numeric.matrix.stride;
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_ARRAY)
        {
            variable.flag |= ShaderVariableFlagBits::Array;
            variable.arrayStride = type->traits.array.stride;
            variable.arrayDimensionsCount = litl::min(type->traits.array.dims_count, ShaderVariable::MaxArrayDimensions);

            for (uint32_t i = 0u; i < variable.arrayDimensionsCount; ++i)
            {
                variable.arrayDimensions[i] = type->traits.array.dims[i];
            }
        }

        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_VOID) { variable.flag |= ShaderVariableFlagBits::Void; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_IMAGE) { variable.flag |= ShaderVariableFlagBits::Image; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLER) { variable.flag |= ShaderVariableFlagBits::Sampler; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_SAMPLED_IMAGE) { variable.flag |= ShaderVariableFlagBits::SampledImage; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_BLOCK) { variable.flag |= ShaderVariableFlagBits::Block; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_EXTERNAL_ACCELERATION_STRUCTURE) { variable.flag |= ShaderVariableFlagBits::Acceleration; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT) { variable.flag |= ShaderVariableFlagBits::Struct; }
        if (type->type_flags & SPV_REFLECT_TYPE_FLAG_REF) { variable.flag |= ShaderVariableFlagBits::Ref; }

        if (variable.scalarSize > 0u)
        {
            variable.scalarSize /= 8u;        // .width is given in bits not bytes
        }

        return variable;
    }

    ResourceProperty resourcePropertyFromSpvReflectBlockVariable(SpvReflectBlockVariable const& block) noexcept
    {
        auto* name = block.name ? block.name
            : (block.type_description && block.type_description->struct_member_name) ? block.type_description->struct_member_name
            : nullptr;

        ResourceProperty property{
            .variable = shaderVariableFromSpvReflectTypeDescription(block.type_description),
            .offset = block.absolute_offset,
            .size = block.size,
            .sizePadded = block.padded_size,
            .name = (name != nullptr ? name : "")
        };

        if (!property.name.empty())
        {
            property.hashedName = StringId(property.name);
        }

        return property;
    }
}