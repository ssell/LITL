#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/shaderModule.hpp"

namespace litl::vulkan
{
    litl::ShaderModule* createShaderModule(VkDevice vkDevice, litl::ShaderModuleDescriptor const& descriptor)
    {
        auto* shaderModuleHandle = new ShaderModuleHandle{
            .vkDevice = vkDevice,
            .vkShaderModule = VK_NULL_HANDLE
        };

        return new litl::ShaderModule(
            &VulkanShaderModuleOperations,
            LITL_PACK_HANDLE(litl::ShaderModuleHandle, shaderModuleHandle),
            descriptor);
    }

    bool build(litl::ShaderModuleDescriptor const& descriptor, litl::ShaderModuleHandle const& litlHandle, litl::ShaderReflection const* pReflection)
    {
        auto* handle = LITL_UNPACK_HANDLE(ShaderModuleHandle, litlHandle);

        if (handle->vkShaderModule != VK_NULL_HANDLE)
        {
            // already exists
            return true;
        }

        const VkShaderModuleCreateInfo shaderModuleInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = descriptor.bytes.size(),
            .pCode = reinterpret_cast<const uint32_t*>(descriptor.bytes.data()),
        };

        const VkResult result = vkCreateShaderModule(handle->vkDevice, &shaderModuleInfo, nullptr, &handle->vkShaderModule);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Shader Module with result ", result);
            return false;
        }

        // For use when creating the graphics pipeline (vkCreateGraphicsPipeline)
        handle->vkShaderStageInfo = VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = convertToVkShaderStage(pReflection->stage),
            .module = handle->vkShaderModule,
            .pName = descriptor.entryPoint.data()
        };

        return true;
    }

    void destroy(litl::ShaderModuleHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(ShaderModuleHandle, litlHandle);

        if (handle->vkShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(handle->vkDevice, handle->vkShaderModule, nullptr);
        }

        delete handle;
    }
}