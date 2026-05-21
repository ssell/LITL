#ifndef LITL_RENDERER_VULKAN_SHADER_MODULE_H__
#define LITL_RENDERER_VULKAN_SHADER_MODULE_H__

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer/resources/shaderModule.hpp"

namespace litl::vulkan
{
    struct ShaderModuleResource
    {
        VkShaderModule vkShaderModule = VK_NULL_HANDLE;
        VkPipelineShaderStageCreateInfo vkCreateInfo{};
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