#include <cstdint>
#include <optional>
#include <span>
#include <spirv_reflect.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer/pipeline/shaderReflection.hpp"

namespace LITL::Renderer
{
    ShaderResourceType fromSpvReflectResourceType(SpvReflectDescriptorType descriptorType);
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
        uint32_t resourceBindingsCount;
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
        uint32_t pushConstantBlocksCount;
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
        return true;
    }

    bool reflectFragmentOutputs(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule)
    {
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
}