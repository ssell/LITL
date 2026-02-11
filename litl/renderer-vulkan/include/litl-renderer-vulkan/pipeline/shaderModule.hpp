#ifndef LITL_VULKAN_RENDERER_SHADER_MODULE_H__
#define LITL_VULKAN_RENDERER_SHADER_MODULE_H__

#include <vulkan/vulkan.h>
#include "litl-renderer/pipeline/shaderModule.hpp"

namespace LITL::Vulkan::Renderer
{
    LITL::Renderer::ShaderModule* createShaderModule(VkDevice vkDevice, LITL::Renderer::ShaderModuleDescriptor const& descriptor);

    bool build(LITL::Renderer::ShaderModuleDescriptor const& descriptor, LITL::Renderer::ShaderModuleHandle const& litlHandle, LITL::Renderer::ShaderReflection const* pReflection);
    void destroy(LITL::Renderer::ShaderModuleHandle const& litlHandle);

    constexpr VkShaderStageFlagBits convertToVkShaderStage(LITL::Renderer::ShaderStage stage)
    {
        switch (stage)
        {
        case LITL::Renderer::ShaderStage::Vertex:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;

        case LITL::Renderer::ShaderStage::Fragment:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;

        case LITL::Renderer::ShaderStage::Geometry:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;

        case LITL::Renderer::ShaderStage::TessellationControl:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;

        case LITL::Renderer::ShaderStage::TessellationEvaluation:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;

        case LITL::Renderer::ShaderStage::Compute:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;

        case LITL::Renderer::ShaderStage::Mesh:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT;

        case LITL::Renderer::ShaderStage::Task:
            return VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT;

        case LITL::Renderer::ShaderStage::Unknown:
        default:
            return static_cast<VkShaderStageFlagBits>(0);
        }
    }

    const LITL::Renderer::ShaderModuleOperations VulkanShaderModuleOperations = {
        &build,
        &destroy
    };
}

#endif