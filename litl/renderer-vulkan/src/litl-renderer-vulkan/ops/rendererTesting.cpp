#include "litl-core/assert.hpp"
#include "litl-renderer-vulkan/renderer.hpp"

namespace litl::vulkan
{
    bool testPipelineLayoutCache(litl::RendererContext* context, ShaderModuleHandle vertexShader, std::string const& vertexEntryPoint, ShaderModuleHandle fragmentShader, std::string const& fragmentEntryPoint) noexcept
    {
        auto* vulkanContext = unwrap(context);

        // 1) Fetch the shader module resources
        auto* vertexShaderResource = vulkanContext->resources.getShaderModule(vertexShader);
        auto* fragmentShaderResource = vulkanContext->resources.getShaderModule(fragmentShader);

        LITL_ASSERT_MSG(vertexShaderResource != nullptr, "Failed to retrieve ShaderModuleResource for Vertex Shader", false);
        LITL_ASSERT_MSG(fragmentShaderResource != nullptr, "Failed to retrieve ShaderModuleResource for Fragment Shader", false);

        // 2) Create the PipelineLayoutDescriptor
        const PipelineLayoutDescriptorCreateInfo createInfo{
            .stages = {
                PipelineLayoutDescriptorShaderModuleInfo {
                    .resource = vertexShaderResource,
                    .stage = ShaderStage::Vertex,
                    .entryPoint = vertexEntryPoint
                },
                PipelineLayoutDescriptorShaderModuleInfo {
                    .resource = fragmentShaderResource,
                    .stage = ShaderStage::Fragment,
                    .entryPoint = fragmentEntryPoint
                }
            }
        };

        PipelineLayoutDescriptor pipelineLayoutDesc{};
        const auto createResult = createPipelineLayoutDescriptor(createInfo, pipelineLayoutDesc);

        LITL_ASSERT_MSG(createResult == MergeShaderReflectionResult::Success, "Failed to create pipeline layout descriptor", false);

        // 3) Get/Create the VkPipelineLayout
        VkPipelineLayout vkPipelineLayout0 = vulkanContext->resources.getOrCreatePipelineLayout(pipelineLayoutDesc);
        LITL_ASSERT_MSG(vkPipelineLayout0 != VK_NULL_HANDLE, "Failed to create VkPipelineLayout 0", false);

        // 4) Create again
        VkPipelineLayout vkPipelineLayout1 = vulkanContext->resources.getOrCreatePipelineLayout(pipelineLayoutDesc);
        LITL_ASSERT_MSG(vkPipelineLayout1 != VK_NULL_HANDLE, "Failed to create VkPipelineLayout 1", false);

        // 5) Verify the same
        LITL_ASSERT_MSG(vkPipelineLayout0 == vkPipelineLayout1, "Mismatch between VkPipelineLayouts - cache miss!", false);

        return true;
    }
}