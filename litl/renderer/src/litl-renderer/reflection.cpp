#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <spirv_reflect.h>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-renderer/reflection.hpp"

/**
 * Reflection is done using the SPIRV-Reflect library to reflect from the raw SPIR-V bytecode.
 * The results of said reflection are then transposed into our common litl::ShaderReflection data structure.
 */

namespace litl
{
    ShaderResourceType fromSpvReflectResourceType(SpvReflectDescriptorType descriptorType);
    void reflectIntoInputOutputVariable(ShaderInputOutputVariable* inputOutputVariable, SpvReflectInterfaceVariable* interfaceVariable);
    ShaderScalarType fromSpvReflectTypeFlagBits(uint32_t typeFlag);

    SpvReflectEntryPoint* selectEntryPoint(const char* entryPoint, SpvReflectShaderModule const* reflectedModule);
    bool reflectShaderStage(EntryPointReflection& litlReflection, SpvReflectEntryPoint const* entryPoint);
    bool reflectResourceBindings(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);
    bool reflectPushConstants(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint);
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
            logError("SPIRV reflection failed with result ", reflectionResult);
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
                reflectPushConstants(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectVertexInputs(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectFragmentOutputs(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint) &&
                reflectComputeInfo(entryPointReflection, &reflectedSpvModule, reflectedSpvEntryPoint))
            {
                shaderReflection.entryPoints.push_back(entryPointReflection);
            }
            else
            {
                logError("SPIRV reflection failed to find desired entry point ", entryPointReflection.entryPoint);
            }
        }

        reflectSpecializationConstants(shaderReflection, &reflectedSpvModule);
        spvReflectDestroyShaderModule(&reflectedSpvModule);

        if (shaderReflection.entryPoints.size() == 0)
        {
            logWarning("SPIRV reflection of shader resulted in 0 entry points.");
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
            logError("SPIRV reflection of unsupported shader stage ", entryPoint->shader_stage);
            return false;
        }

        return true;
    }

    bool reflectResourceBindings(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        uint32_t resourceBindingsCount = 0;
        auto result = spvReflectEnumerateEntryPointDescriptorBindings(reflectedModule, entryPoint->name, &resourceBindingsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate resource binding count with result ", result);
            return false;
        }

        // we piggy-back off of vector here to ensure the memory storing the pointers is freed at the end of scope (failure or success)
        std::vector<SpvReflectDescriptorBinding*> resourceBindings(resourceBindingsCount);
        result = spvReflectEnumerateEntryPointDescriptorBindings(reflectedModule, entryPoint->name, &resourceBindingsCount, resourceBindings.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate resource bindings with result ", result);
            return false;
        }

        litlReflection.resources.reserve(resourceBindingsCount);

        for (uint32_t i = 0; i < resourceBindingsCount; ++i)
        {
            auto binding = *resourceBindings[i];

            if (binding.set >= 32u)
            {
                // Set indices >= 32 are invalid due to the DescriptorSetChangeTracker.
                // If we need to support more than 32 sets, then that is where the change will need to be done.
                logError("Shader reflection processing encountered binding set index = ", binding.set, ". Only set indices [0, 31] are valid.");
                return false;
            }

            litlReflection.resources.push_back(ResourceBinding{
                    .type = fromSpvReflectResourceType(binding.descriptor_type),
                    .set = binding.set,
                    .binding = binding.binding,
                    .arraySize = binding.count,
                    .sizeBytes = binding.block.size,
                    .name = binding.name
                });
        }

        return true;
    }

    bool reflectPushConstants(EntryPointReflection& litlReflection, SpvReflectShaderModule const* reflectedModule, SpvReflectEntryPoint const* entryPoint)
    {
        uint32_t pushConstantBlocksCount = 0;
        auto result = spvReflectEnumerateEntryPointPushConstantBlocks(reflectedModule, entryPoint->name, &pushConstantBlocksCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate push constants block count with result ", result);
            return false;
        }

        std::vector<SpvReflectBlockVariable*> pushConstantBlocks(pushConstantBlocksCount);
        result = spvReflectEnumerateEntryPointPushConstantBlocks(reflectedModule, entryPoint->name, &pushConstantBlocksCount, pushConstantBlocks.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate push constant blocks with result ", result);
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
            logError("SPIRV reflection failed to enumerate input variable count with result ", result);
            return false;
        }

        std::vector<SpvReflectInterfaceVariable*> inputVariables(vertexInputsCount);
        result = spvReflectEnumerateEntryPointInputVariables(reflectedModule, entryPoint->name, &vertexInputsCount, inputVariables.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate input variables with result ", result);
            return false;
        }

        litlReflection.vertexInputs.reserve(vertexInputsCount);

        for (uint32_t i = 0; i < vertexInputsCount; ++i)
        {
            auto inputVariable = *inputVariables[i];

            litlReflection.vertexInputs.push_back(ShaderInputOutputVariable{
                    .location = inputVariable.location,
                    .name = (inputVariable.name != nullptr ? inputVariable.name : "")
                });

            reflectIntoInputOutputVariable(&litlReflection.vertexInputs[i], &inputVariable);
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
            logError("SPIRV reflection failed to enumerate fragment output variable count with result ", result);
            return false;
        }

        std::vector<SpvReflectInterfaceVariable*> outputVariables(fragmentOutputsCount);
        result = spvReflectEnumerateEntryPointOutputVariables(reflectedModule, entryPoint->name, &fragmentOutputsCount, outputVariables.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate fragment output variables with result ", result);
            return false;
        }

        litlReflection.fragmentOutputs.reserve(fragmentOutputsCount);

        for (uint32_t i = 0; i < fragmentOutputsCount; ++i)
        {
            auto outputVariable = *outputVariables[i];

            litlReflection.fragmentOutputs.push_back(ShaderInputOutputVariable{
                    .location = outputVariable.location,
                    .name = (outputVariable.name != nullptr ? outputVariable.name : "")
                });

            reflectIntoInputOutputVariable(&litlReflection.fragmentOutputs[i], &outputVariable);
        }

        return true;
    }

    bool reflectSpecializationConstants(ShaderReflection& litlReflection, SpvReflectShaderModule const* reflectedModule)
    {
        uint32_t constantsCount = 0;
        auto result = spvReflectEnumerateSpecializationConstants(reflectedModule, &constantsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate specialization cosntants count with result ", result);
            return false;
        }

        std::vector<SpvReflectSpecializationConstant*> constants(constantsCount);
        result = spvReflectEnumerateSpecializationConstants(reflectedModule, &constantsCount, constants.data());

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate specialization constants with result ", result);
            return false;
        }

        litlReflection.specializationConstants.reserve(constantsCount);

        for (uint32_t i = 0; i < constantsCount; ++i)
        {
            auto constant = *constants[i];

            litlReflection.specializationConstants.push_back(SpecializationConstant{
                    .id = constant.constant_id,
                    .scalarType = fromSpvReflectTypeFlagBits(constant.type_description->type_flags),
                    .name = (constant.name != nullptr ? constant.name : "")
                });
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

    void reflectIntoInputOutputVariable(ShaderInputOutputVariable* inputOutputVariable, SpvReflectInterfaceVariable* interfaceVariable)
    {
        switch (interfaceVariable->format)
        {
        case SPV_REFLECT_FORMAT_UNDEFINED:
            inputOutputVariable->scalarType = ShaderScalarType::Unknown;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R16_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R16_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R16_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R16G16_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R16G16_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R16G16_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16A16_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16A16_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R32_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R32_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R32_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R32G32_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R32G32_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R64_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R64_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R64_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 1;
            break;
        case SPV_REFLECT_FORMAT_R64G64_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R64G64_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R64G64_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 2;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 3;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64A64_UINT:
            inputOutputVariable->scalarType = ShaderScalarType::Uint;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64A64_SINT:
            inputOutputVariable->scalarType = ShaderScalarType::Int;
            inputOutputVariable->componentCount = 4;
            break;
        case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT:
            inputOutputVariable->scalarType = ShaderScalarType::Float;
            inputOutputVariable->componentCount = 4;
            break;

        default:
            logWarning("Unhandled SPIR-V input/output variable format: ", static_cast<uint32_t>(interfaceVariable->format));
            break;
        }
    }

    ShaderScalarType fromSpvReflectTypeFlagBits(uint32_t typeFlag)
    {
        if (typeFlag & SPV_REFLECT_TYPE_FLAG_BOOL)
        {
            return ShaderScalarType::Bool;
        }
        else if (typeFlag & SPV_REFLECT_TYPE_FLAG_INT)
        {
            return ShaderScalarType::Int;
        }
        else if (typeFlag & SPV_REFLECT_TYPE_FLAG_FLOAT)
        {
            return ShaderScalarType::Float;
        }

        return ShaderScalarType::Unknown;
    }
}