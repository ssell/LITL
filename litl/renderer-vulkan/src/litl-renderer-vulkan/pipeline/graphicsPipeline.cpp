#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/pipeline/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"
#include "litl-renderer-vulkan/pipeline/shaderModule.hpp"

namespace LITL::Vulkan::Renderer
{
    LITL::Renderer::GraphicsPipeline* createGraphicsPipeline(VkDevice vkDevice, VkFormat vkSwapChainImageFormat, LITL::Renderer::GraphicsPipelineDescriptor const& descriptor)
    {
        auto* graphicsPipelineHandle = new GraphicsPipelineHandle{
            .vkDevice = vkDevice,
            .vkSwapChainImageFormat = vkSwapChainImageFormat,
            .vkPipeline = VK_NULL_HANDLE
        };

        return new LITL::Renderer::GraphicsPipeline(
            &VulkanGraphicsPipelineOperations,
            LITL_PACK_HANDLE(LITL::Renderer::GraphicsPipelineHandle, graphicsPipelineHandle),
            descriptor
        );
    }

    bool build(LITL::Renderer::GraphicsPipelineDescriptor const& descriptor, LITL::Renderer::GraphicsPipelineHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(GraphicsPipelineHandle, litlHandle);

        // Not using buffers atm (baked into shader), update when using buffers.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        // Pull from pipeline?
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        std::vector<VkPipelineShaderStageCreateInfo> allShaderStages;
        allShaderStages.reserve(8);

        if (descriptor.pVertexShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pVertexShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pFragmentShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pFragmentShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pGeometryShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pGeometryShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pTessellationControlShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pTessellationControlShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pTessellationEvaulationShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pTessellationEvaulationShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pTaskShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pTaskShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        if (descriptor.pMeshShader != nullptr)
        {
            auto* shaderHandle = LITL_UNPACK_HANDLE_PTR(ShaderModuleHandle, descriptor.pMeshShader->getHandle());
            allShaderStages.emplace_back(shaderHandle->vkShaderStageInfo);
        }

        const auto pipelineRenderingInfo = VkPipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &handle->vkSwapChainImageFormat
        };

        const auto pipelineHandle = LITL_UNPACK_HANDLE_PTR(PipelineLayoutHandle, descriptor.pPipelineLayout->getHandle());

        const auto pipelineInfo = VkGraphicsPipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingInfo,
            .stageCount = static_cast<uint32_t>(allShaderStages.size()),
            .pStages = allShaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,              // Optional
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = pipelineHandle->vkPipelineLayout,
            .renderPass = nullptr,                      // Not used with dynamic rendering
            .subpass = 0,                               // Not currently used
            .basePipelineHandle = nullptr,              // Optional
            .basePipelineIndex = -1,                    // Optional
        };

        const auto result = vkCreateGraphicsPipelines(handle->vkDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle->vkPipeline);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Graphics Pipeline with result ", result);
            return false;
        }

        return true;
    }

    void destroy(LITL::Renderer::GraphicsPipelineHandle const& litlHandle)
    {
        auto* handle = LITL_UNPACK_HANDLE(GraphicsPipelineHandle, litlHandle);

        if (handle->vkPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(handle->vkDevice, handle->vkPipeline, nullptr);
        }

        delete handle;
    }
}