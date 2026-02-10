#include <cstdint>
#include <optional>
#include <span>
#include <spirv_reflect.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer/pipeline/shaderReflection.hpp"

/**
 * Reflection is done using the SPIRV-Reflect library to reflect from the raw SPIR-V bytecode.
 * The results of said reflection are then transposed into our common LITL::Renderer::ShaderReflection data structure.
 */

namespace LITL::Renderer
{
    ShaderResourceType fromSpvReflectResourceType(SpvReflectDescriptorType descriptorType);
    void reflectIntoInputOutputVariable(ShaderInputOutputVariable* inputOutputVariable, SpvReflectInterfaceVariable* interfaceVariable);

    bool reflectShaderStage(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);
    bool reflectResourceBindings(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);
    bool reflectPushConstants(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);
    bool reflectVertexInputs(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);
    bool reflectFragmentOutputs(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);

    std::optional<ShaderReflection> reflectSPIRV(std::span<uint8_t const> spirvByteCode)
    {
        std::optional<ShaderReflection> returnVal = std::nullopt;
        SpvReflectShaderModule module{};

        auto result = spvReflectCreateShaderModule(spirvByteCode.size_bytes(), spirvByteCode.data(), &module);

        if (result == SPV_REFLECT_RESULT_SUCCESS)
        {
            ShaderReflection reflected{};
            reflected.stage = static_cast<ShaderStage>(module.shader_stage);

            if (reflectShaderStage(&reflected, &module) &&
                reflectResourceBindings(&reflected, &module) &&
                reflectVertexInputs(&reflected, &module) &&
                reflectFragmentOutputs(&reflected, &module))
            {
                returnVal = reflected;
            }
        }
        else
        {
            logError("SPIRV reflection failed with result ", result);
        }

        spvReflectDestroyShaderModule(&module);
        return returnVal;
    }

    bool reflectShaderStage(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
        litlReflection->stage = static_cast<ShaderStage>(reflectedModule->shader_stage);
        return true;
    }

    bool reflectResourceBindings(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
        uint32_t resourceBindingsCount = 0;
        auto result = spvReflectEnumerateDescriptorBindings(reflectedModule, &resourceBindingsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate resource binding count with result ", result);
            return false;
        }

        // Note we malloc intentionally. SPIRV-Reflect is a C library and uses malloc/free internally. The call to spvReflectDestroyShaderModule calls free on our dynamic resources.
        SpvReflectDescriptorBinding** resourceBindings = (SpvReflectDescriptorBinding**)malloc(resourceBindingsCount * sizeof(SpvReflectDescriptorBinding*));
        result = spvReflectEnumerateDescriptorBindings(reflectedModule, &resourceBindingsCount, resourceBindings);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate resource bindings with result ", result);
            return false;
        }

        litlReflection->resources.reserve(resourceBindingsCount);

        for (uint32_t i = 0; i < resourceBindingsCount; ++i)
        {
            auto binding = *resourceBindings[i];

            litlReflection->resources.push_back(ResourceBinding{
                    .name = binding.name,
                    .type = fromSpvReflectResourceType(binding.descriptor_type),
                    .set = binding.set,
                    .binding = binding.binding,
                    .arraySize = binding.count,
                    .sizeBytes = binding.block.size
                });
        }

        return true;
    }

    bool reflectPushConstants(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
        uint32_t pushConstantBlocksCount = 0;
        auto result = spvReflectEnumeratePushConstantBlocks(reflectedModule, &pushConstantBlocksCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate push constants block count with result ", result);
            return false;
        }

        SpvReflectBlockVariable** pushConstantBlocks = (SpvReflectBlockVariable**)malloc(pushConstantBlocksCount * sizeof(SpvReflectBlockVariable*));
        result = spvReflectEnumeratePushConstantBlocks(reflectedModule, &pushConstantBlocksCount, pushConstantBlocks);
        
        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate push constant blocks with result ", result);
            return false;
        }

        litlReflection->pushConstants.reserve(pushConstantBlocksCount);

        for (uint32_t i = 0; i < pushConstantBlocksCount; ++i)
        {
            auto pushConstantBlock = *pushConstantBlocks[i];

            litlReflection->pushConstants.push_back(PushConstantRange{
                    .offset = pushConstantBlock.offset,
                    .sizeBytes = pushConstantBlock.size
                });
        }

        return true;
    }

    bool reflectVertexInputs(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
        uint32_t vertexInputsCount = 0;
        auto result = spvReflectEnumerateInputVariables(reflectedModule, &vertexInputsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate input variable count with result ", result);
            return false;
        }

        SpvReflectInterfaceVariable** inputVariables = (SpvReflectInterfaceVariable**)malloc(vertexInputsCount * sizeof(SpvReflectInterfaceVariable*));
        result = spvReflectEnumerateInputVariables(reflectedModule, &vertexInputsCount, inputVariables);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate input variables with result ", result);
            return false;
        }

        litlReflection->vertexInputs.reserve(vertexInputsCount);

        for (uint32_t i = 0; i < vertexInputsCount; ++i)
        {
            auto inputVariable = *inputVariables[i];

            litlReflection->vertexInputs.push_back(ShaderInputOutputVariable{
                    .name = inputVariable.name,
                    .location = inputVariable.location
                });

            reflectIntoInputOutputVariable(&litlReflection->vertexInputs[i], &inputVariable);
        }

        return true;
    }

    bool reflectFragmentOutputs(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
        uint32_t fragmentOutputsCount = 0;
        auto result = spvReflectEnumerateOutputVariables(reflectedModule, &fragmentOutputsCount, nullptr);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate fragment output variable count with result ", result);
            return false;
        }

        SpvReflectInterfaceVariable** outputVariables = (SpvReflectInterfaceVariable**)malloc(fragmentOutputsCount * sizeof(SpvReflectInterfaceVariable*));
        result = spvReflectEnumerateInputVariables(reflectedModule, &fragmentOutputsCount, outputVariables);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate fragment output variables with result ", result);
            return false;
        }

        litlReflection->fragmentOutputs.reserve(fragmentOutputsCount);

        for (uint32_t i = 0; i < fragmentOutputsCount; ++i)
        {
            auto outputVariable = *outputVariables[i];

            litlReflection->fragmentOutputs.push_back(ShaderInputOutputVariable{
                    .name = outputVariable.name,
                    .location = outputVariable.location
                });

            reflectIntoInputOutputVariable(&litlReflection->fragmentOutputs[i], &outputVariable);
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
        }
    }
}