#include <cstdint>
#include <optional>
#include <span>
#include <spirv_reflect.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer/pipeline/shaderReflection.hpp"

namespace LITL::Renderer
{
    bool reflectShaderStage(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);
    bool reflectResourceBindings(ShaderReflection* litlReflection, SpvReflectShaderModule* reflectedModule);

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
                reflectResourceBindings(&reflected, &module))
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

        // Note here we use malloc intentionally. The call to spvReflectDestroyShaderModule `free`s all of these dynamically allocated resources.
        SpvReflectDescriptorBinding** resourceBindings = (SpvReflectDescriptorBinding**)malloc(resourceBindingsCount * sizeof(SpvReflectDescriptorBinding*));
        result = spvReflectEnumerateDescriptorBindings(reflectedModule, &resourceBindingsCount, resourceBindings);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {
            logError("SPIRV reflection failed to enumerate resource bindings with result ", result);
            return false;
        }

        // ...

        return true;
    }
}