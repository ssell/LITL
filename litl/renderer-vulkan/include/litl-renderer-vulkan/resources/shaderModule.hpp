#ifndef LITL_RENDERER_VULKAN_SHADER_MODULE_H__
#define LITL_RENDERER_VULKAN_SHADER_MODULE_H__

#include <optional>

#include "litl-core/stringId.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-renderer/resources/shaderModule.hpp"
#include "litl-renderer/reflection.hpp"

namespace litl::vulkan
{
    struct ShaderModuleResource
    {
        /// <summary>
        /// Handle to a compiled shader module entry point.
        /// </summary>
        VkShaderModule vkShaderModule = VK_NULL_HANDLE;

        /// <summary>
        /// Results of reflection on the SPIR-V bytecode.
        /// </summary>
        ShaderReflection reflection;

        /// <summary>
        /// The hash of the SPIR-V bytecode. Used for shader invalidation.
        /// </summary>
        uint64_t spirvHash;

        /// <summary>
        /// The hashed ID of the resource string.
        /// </summary>
        StringId resourceId;

        /// <summary>
        /// The name of the resource from which this shader module was loaded.
        /// </summary>
        std::string resource;

        /// <summary>
        /// Given an entry point name, creates a corresponding VkPipelineShaderStageCreateInfo if it exists.
        /// </summary>
        /// <param name="entryPoint"></param>
        /// <param name="spec"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<VkPipelineShaderStageCreateInfo> createStageInfo(char const* entryPointName, VkSpecializationInfo const* spec = nullptr) const noexcept
        {
            auto entryPoint = reflection.getEntryPoint(entryPointName);

            if (!entryPoint.has_value())
            {
                return std::nullopt;
            }

            return VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = static_cast<VkShaderStageFlagBits>(toVkShaderStageFlags(entryPoint.value()->stage)),
                .module = vkShaderModule,
                .pName = entryPointName,
                .pSpecializationInfo = spec,
            };
        }
    };
}

#endif