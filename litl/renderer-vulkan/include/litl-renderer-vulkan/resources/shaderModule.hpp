#ifndef LITL_RENDERER_VULKAN_SHADER_MODULE_H__
#define LITL_RENDERER_VULKAN_SHADER_MODULE_H__

#include "litl-renderer-vulkan/common.hpp"
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
        /// The shader stage that the module is for.
        /// </summary>
        VkShaderStageFlagBits stage;

        /// <summary>
        /// Results of reflection on the SPIR-V bytecode.
        /// </summary>
        ShaderReflection reflection;

        /// <summary>
        /// The hash of the SPIR-V bytecode. Used for shader invalidation.
        /// </summary>
        uint64_t spirvHash;

        /// <summary>
        /// Key for this module in the resource map.
        /// </summary>
        std::string resourceMapKey;

        VkPipelineShaderStageCreateInfo createStageInfo(char const* entryPoint, VkSpecializationInfo const* spec = nullptr) const noexcept
        {
            return VkPipelineShaderStageCreateInfo {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = stage,
                .module = vkShaderModule,
                .pName = entryPoint,
                .pSpecializationInfo = spec,
            };
        }
    };

    constexpr VkShaderStageFlagBits convertToVkShaderStage(litl::ShaderStage stage)
    {
        switch (stage)
        {
        case litl::ShaderStage::Vertex:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;

        case litl::ShaderStage::Fragment:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;

        case litl::ShaderStage::Geometry:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;

        case litl::ShaderStage::TessellationControl:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

        case litl::ShaderStage::TessellationEvaluation:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

        case litl::ShaderStage::Compute:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;

        case litl::ShaderStage::Mesh:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;

        case litl::ShaderStage::Task:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT;

        case litl::ShaderStage::None:
        default:
            return static_cast<VkShaderStageFlagBits>(0);
        }
    }
}

#endif