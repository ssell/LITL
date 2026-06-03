#include <array>

#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-renderer/reflection.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/hash.hpp"

namespace litl::vulkan
{
    void ResourceManager::build(RendererContext& context) noexcept
    {
        m_pContext = &context;
        m_pipelineLayoutCache.build(context.device.vkDevice);
    }

    void ResourceManager::destroy() noexcept
    {
        // Graphics Pipelines
        std::vector<GraphicsPipelineHandle> graphicsPipelineHandles;
        m_graphicsPipelinePool.getAllHandles(graphicsPipelineHandles);

        for (auto graphicsPipelineHandle : graphicsPipelineHandles)
        {
            destroyGraphicsPipeline(graphicsPipelineHandle);
        }

        // Command Buffers
        std::vector<CommandBufferHandle> commandBufferHandles;
        m_commandBufferPool.getAllHandles(commandBufferHandles);

        for (auto commandBufferHandle : commandBufferHandles)
        {
            destroyCommandBuffer(commandBufferHandle);
        }

        // Pipeline Layouts
        m_pipelineLayoutCache.destroy();

        // Shader Modules
        for (auto shaderModuleHandleKvp : m_shaderModuleMap)
        {
            destroyShaderModule(shaderModuleHandleKvp.second);
        }

    }

    //--------------------------------------------------------------------------------------
    // Buffer
    //--------------------------------------------------------------------------------------

    BufferHandle ResourceManager::createBuffer(BufferDescriptor const& descriptor) noexcept
    {
        // ... todo ...

        return BufferHandle{};
    }

    BufferResource* ResourceManager::getBuffer(BufferHandle handle) noexcept
    {
        return m_bufferPool.get(handle);
    }

    void ResourceManager::destroyBuffer(BufferHandle handle) noexcept
    {
        if (m_bufferPool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // CommandBuffer
    //--------------------------------------------------------------------------------------

    CommandBufferHandle ResourceManager::createCommandBuffer(CommandBufferDescriptor const& descriptor) noexcept
    {
        CommandBufferResource resource{};

        const VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = m_pContext->device.vkCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1u
        };

        const VkResult result = vkAllocateCommandBuffers(m_pContext->device.vkDevice, &allocateInfo, &resource.vkCommandBuffer);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan CommandBuffer with result ", result);
            return {};
        }

        return m_commandBufferPool.create(resource);
    }

    CommandBufferResource* ResourceManager::getCommandBuffer(CommandBufferHandle handle) noexcept
    {
        return m_commandBufferPool.get(handle);
    }

    void ResourceManager::destroyCommandBuffer(CommandBufferHandle handle) noexcept
    {
        CommandBufferResource* resource = m_commandBufferPool.get(handle);

        if (resource != nullptr)
        {
            if (resource->vkCommandBuffer != VK_NULL_HANDLE)
            {
                vkFreeCommandBuffers(m_pContext->device.vkDevice, m_pContext->device.vkCommandPool, 1, &resource->vkCommandBuffer);
            }

            m_commandBufferPool.destroy(handle);
        }
    }

    //--------------------------------------------------------------------------------------
    // ComputePipeline
    //--------------------------------------------------------------------------------------

    /// <summary>
    /// Creates the resource.
    /// This is split out because two paths need to be able to create a compute pipeline resource: createComputePipeline and onShaderModuleReload.
    /// </summary>
    /// <param name="resources"></param>
    /// <param name="context"></param>
    /// <param name="resource"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    bool createComputePipelineResource(ResourceManager& resources, RendererContext* context, ComputePipelineResource& resource, ComputePipelineDescriptor const& descriptor)
    {
        // ... todo ...

        return true;
    }

    ComputePipelineHandle ResourceManager::createComputePipeline(ComputePipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        // ... remember to update shader module reference map ...
        return ComputePipelineHandle{};
    }

    ComputePipelineResource* ResourceManager::getComputePipeline(ComputePipelineHandle handle) noexcept
    {
        return m_computePipelinePool.get(handle);
    }

    void ResourceManager::destroyComputePipeline(ComputePipelineHandle handle) noexcept
    {
        if (m_computePipelinePool.destroy(handle))
        {
            // ... todo ...
            // ... remember to update shader module reference map ...
        }
    }

    //--------------------------------------------------------------------------------------
    // GraphicsPipeline
    //--------------------------------------------------------------------------------------

    void createPipelineShaderStageCreateInfo(ShaderModuleResource* resource, PipelineShaderDescriptor const& descriptor, std::array<VkPipelineShaderStageCreateInfo, 7>& stages, uint32_t& count, PipelineLayoutDescriptorCreateInfo& pipelineLayoutDescriptorCreateInfo) noexcept
    {
        if (resource == nullptr)
        {
            return;
        }

        auto entryPoint = resource->reflection.getEntryPoint(descriptor.entryPoint);

        if (!entryPoint.has_value() || (*entryPoint) == nullptr)
        {
            logWarning("Vulkan Graphics Pipeline creation requested invalid shader with entry point of '", descriptor.entryPoint, "'");
            return;
        }

        stages[count++] = VkPipelineShaderStageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = static_cast<VkShaderStageFlagBits>(toVkShaderStageFlags(descriptor.stage)),
                .module = resource->vkShaderModule,
                .pName = descriptor.entryPoint.c_str(),
                .pSpecializationInfo = nullptr
        };

        pipelineLayoutDescriptorCreateInfo.stages.push_back(PipelineLayoutDescriptorShaderModuleInfo{
            .resource = resource,
            .stage = descriptor.stage,
            .entryPoint = descriptor.entryPoint
        });
    }

    VkPipelineVertexInputStateCreateInfo createVertexInputStateCreateInfo(VertexInputState const& vertexInput, std::vector<VkVertexInputBindingDescription>& bindingDescriptions, std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
    {
        bindingDescriptions.reserve(vertexInput.bindings.size());
        attributeDescriptions.reserve(vertexInput.attributes.size());

        for (auto& vertexBinding : vertexInput.bindings)
        {
            bindingDescriptions.push_back(VkVertexInputBindingDescription{
                .binding = vertexBinding.binding,
                .stride = vertexBinding.stride,
                .inputRate = toVkVertexInputRate(vertexBinding.rate)
                });
        }

        for (auto& vertexAttribute : vertexInput.attributes)
        {
            attributeDescriptions.push_back(VkVertexInputAttributeDescription{
                .location = vertexAttribute.location,
                .binding = vertexAttribute.binding,
                .format = toVkFormat(vertexAttribute.format),
                .offset = vertexAttribute.offset
                });
        }

        return VkPipelineVertexInputStateCreateInfo {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
            .pVertexBindingDescriptions = bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };
    }

    /// <summary>
    /// Creates the resource.
    /// This is split out because two paths need to be able to create a graphics pipeline resource: createGraphicsPipeline and onShaderModuleReload.
    /// </summary>
    /// <param name="context"></param>
    /// <param name="resource"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    bool createGraphicsPipelineResource(ResourceManager& resources, RendererContext* context, GraphicsPipelineResource& resource, GraphicsPipelineDescriptor const& descriptor)
    {
        uint32_t shaderStageCount = 0;
        std::array<VkPipelineShaderStageCreateInfo, 7> shaderStages;
        PipelineLayoutDescriptorCreateInfo pipelineLayoutDescriptorCreateInfo{};        // used later, but everything we need to make it is retrieved now.

        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.vertex.handle), descriptor.vertex, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.fragment.handle), descriptor.fragment, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.geometry.handle), descriptor.geometry, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.tessellationEvaluation.handle), descriptor.tessellationEvaluation, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.tessellationControl.handle), descriptor.tessellationControl, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.mesh.handle), descriptor.mesh, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);
        createPipelineShaderStageCreateInfo(resources.getShaderModule(descriptor.task.handle), descriptor.task, shaderStages, shaderStageCount, pipelineLayoutDescriptorCreateInfo);

        // ---- Vertex Input

        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

        const VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = createVertexInputStateCreateInfo(descriptor.vertexInput, vertexInputBindingDescriptions, vertexInputAttributeDescriptions);

        // ---- Input Assembly

        const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = toVkPrimitiveTopology(descriptor.inputAssembly.topology),
            .primitiveRestartEnable = descriptor.inputAssembly.primitiveRestartEnabled
        };

        // ---- Tessellation State

        const VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .patchControlPoints = (descriptor.tessellation.has_value() ? descriptor.tessellation->patchControlPoints : 0)
        };

        // ---- Rasterization State

        const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = descriptor.rasterization.depthClampEnabled,
            .rasterizerDiscardEnable = descriptor.rasterization.rasterizerDiscardEnabled,
            .polygonMode = toVkPolygonMode(descriptor.rasterization.polygonMode),
            .cullMode = toVkCullModeFlag(descriptor.rasterization.cullMode),
            .frontFace = toVkFrontFace(descriptor.rasterization.frontFace),
            .depthBiasEnable = descriptor.rasterization.depthBiasEnabled,
            .depthBiasConstantFactor = descriptor.rasterization.depthBiasConstantFactor,
            .depthBiasClamp = descriptor.rasterization.depthBiasClamp,
            .depthBiasSlopeFactor = descriptor.rasterization.depthBiasSlopeFactor,
            .lineWidth = descriptor.rasterization.lineWidth
        };

        // ---- Multisample State

        const VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = static_cast<VkSampleCountFlagBits>(toVkSampleCountFlag(descriptor.multisample.sampleCount)),
            .sampleShadingEnable = descriptor.multisample.sampleShadingEnabled,
            .minSampleShading = descriptor.multisample.minSampleShading,
            .pSampleMask = nullptr,     // not supported yet
            .alphaToCoverageEnable = descriptor.multisample.alphaToCoverageEnabled,
            .alphaToOneEnable = descriptor.multisample.alphaToOneEnabled
        };

        // ---- Depth Stencil State

        const VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = descriptor.depthStencil.depthState.depthTestEnabled,
            .depthWriteEnable = descriptor.depthStencil.depthState.depthWriteEnabled,
            .depthCompareOp = toVkCompareOp(descriptor.depthStencil.depthState.compareOp),
            .depthBoundsTestEnable = descriptor.depthStencil.depthState.depthBoundsTestEnabled,
            .stencilTestEnable = descriptor.depthStencil.stencilState.stencilTestEnabled,
            .front = VkStencilOpState {
                    .failOp = toVkStencilOp(descriptor.depthStencil.stencilState.frontStencilState.failOp),
                    .passOp = toVkStencilOp(descriptor.depthStencil.stencilState.frontStencilState.passOp),
                    .depthFailOp = toVkStencilOp(descriptor.depthStencil.stencilState.frontStencilState.depthFailOp),
                    .compareOp = toVkCompareOp(descriptor.depthStencil.stencilState.frontStencilState.compareOp),
                    .compareMask = descriptor.depthStencil.stencilState.frontStencilState.compareMask,
                    .writeMask = descriptor.depthStencil.stencilState.frontStencilState.writeMask,
                    .reference = descriptor.depthStencil.stencilState.frontStencilState.reference
                },
            .back = VkStencilOpState {
                    .failOp = toVkStencilOp(descriptor.depthStencil.stencilState.backStencilState.failOp),
                    .passOp = toVkStencilOp(descriptor.depthStencil.stencilState.backStencilState.passOp),
                    .depthFailOp = toVkStencilOp(descriptor.depthStencil.stencilState.backStencilState.depthFailOp),
                    .compareOp = toVkCompareOp(descriptor.depthStencil.stencilState.backStencilState.compareOp),
                    .compareMask = descriptor.depthStencil.stencilState.backStencilState.compareMask,
                    .writeMask = descriptor.depthStencil.stencilState.backStencilState.writeMask,
                    .reference = descriptor.depthStencil.stencilState.backStencilState.reference
                },
            .minDepthBounds = descriptor.depthStencil.depthState.minDepthBounds,
            .maxDepthBounds = descriptor.depthStencil.depthState.maxDepthBounds
        };

        // ---- Color Blend State

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
        colorBlendAttachmentStates.reserve(descriptor.colorBlend.colorAttachmentBlendStates.size());

        for (auto& colorBlendAttachment : descriptor.colorBlend.colorAttachmentBlendStates)
        {
            colorBlendAttachmentStates.push_back(VkPipelineColorBlendAttachmentState{
                .blendEnable = colorBlendAttachment.attachmentBlendEnabled,
                .srcColorBlendFactor = toVkBlendFactor(colorBlendAttachment.srcColorBlendFactor),
                .dstColorBlendFactor = toVkBlendFactor(colorBlendAttachment.dstColorBlendFactor),
                .colorBlendOp = toVkBlendOp(colorBlendAttachment.colorBlendOp),
                .srcAlphaBlendFactor = toVkBlendFactor(colorBlendAttachment.srcAlphaBlendFactor),
                .dstAlphaBlendFactor = toVkBlendFactor(colorBlendAttachment.dstAlphaBlendFactor),
                .alphaBlendOp = toVkBlendOp(colorBlendAttachment.alphaBlendOp),
                .colorWriteMask = toVkColorComponentFlag(colorBlendAttachment.colorWriteMask)
                });
        }

        const VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = descriptor.colorBlend.logicOpEnabled,
            .logicOp = toVkLogicOp(descriptor.colorBlend.logicOp),
            .attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size()),
            .pAttachments = colorBlendAttachmentStates.data(),
            .blendConstants = {
                descriptor.colorBlend.blendConstants[0],
                descriptor.colorBlend.blendConstants[1],
                descriptor.colorBlend.blendConstants[2],
                descriptor.colorBlend.blendConstants[3]
            }
        };

        // ---- Dynamic State

        std::vector<VkDynamicState> dynamicStates;
        dynamicStates.reserve(descriptor.dynamicState.states.size() + 2);

        // These dynamic states are always enabled for LITL
        dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);      // dynamically modify both viewport data AND number of viewports
        dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);

        for (auto dynamicState : descriptor.dynamicState.states)
        {
            dynamicStates.push_back(toVkDynamicState(dynamicState));
        }

        const VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
            .pDynamicStates = dynamicStates.data()
        };

        // ---- Pipeline Layout

        PipelineLayoutDescriptor pipelineLayoutDescriptor{};
        const MergeShaderReflectionResult mergePipelineLayoutResult = createPipelineLayoutDescriptor(pipelineLayoutDescriptorCreateInfo, pipelineLayoutDescriptor);

        if (mergePipelineLayoutResult != MergeShaderReflectionResult::Success)
        {
            logError("Failed to create Vulkan Graphics Pipeline due to failed pipeline layout merger with result ", static_cast<uint32_t>(mergePipelineLayoutResult));
            return {};
        }

        const VkPipelineLayout vkPipelineLayout = resources.getOrCreatePipelineLayout(pipelineLayoutDescriptor);

        if (vkPipelineLayout == VK_NULL_HANDLE)
        {
            logError("Failed to create Vulkan Graphics Pipeline due to failure to get or create Pipeline Layout");
            return {};
        }

        // ---- Rendering Info

        std::vector<VkFormat> colorAttachmentFormats;
        colorAttachmentFormats.reserve(descriptor.renderTargets.colorAttachmentCount);

        for (uint32_t i = 0u; i < descriptor.renderTargets.colorAttachmentCount; ++i)
        {
            colorAttachmentFormats.push_back(toVkFormat(descriptor.renderTargets.colorAttachments[i]));
        }

        const VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .pNext = nullptr,
            .viewMask = descriptor.renderTargets.viewMask,           // must match the view mask provided in vkCmdBeginRendering
            .colorAttachmentCount = descriptor.renderTargets.colorAttachmentCount,
            .pColorAttachmentFormats = colorAttachmentFormats.data(),
            .depthAttachmentFormat = toVkFormat(descriptor.renderTargets.depthFormat),
            .stencilAttachmentFormat = toVkFormat(descriptor.renderTargets.stencilFormat)
        };

        // ---- Create the Graphics Pipeline

        /**
         * Note on structure below, several of the provided substructs can be VK_NULL_HANDLE with the following conditions:
         *
         *     .pInputAssemblyState -> NULL if `PRIMITIVE_RESTART_ENABLE` + `PRIMITIVE_TOPOLOGY` (and `dynamicPrimitiveTopologyUnrestricted == VK_TRUE`)
         *     .pTessellationState  -> NULL if `PATCH_CONTROL_POINTS_EXT`
         *     .pViewportState      -> NULL if `VIEWPORT_WITH_COUNT` + `SCISSOR_WITH_COUNT`
         *     .pMultisampleState   -> NULL if `RASTERIZATION_SAMPLES_EXT` + `SAMPLE_MASK_EXT` + `LPHA_TO_COVERAGE_ENABLE_EXT` (+ `ALPHA_TO_ONE_ENABLE_EXT` if `alphaToOne` feature on)
         *     .pDepthStencilState  -> NULL if full set of `DEPTH_TEST_ENABLE`/`DEPTH_WRITE_ENABLE`/`DEPTH_COMPARE_OP`/`DEPTH_BOUNDS_TEST_ENABLE`/`STENCIL_TEST_ENABLE`/`STENCIL_OP` (+ `DEPTH_BOUNDS`)
         */

        const VkGraphicsPipelineCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .flags = 0,
            .stageCount = shaderStageCount,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputCreateInfo,
            .pInputAssemblyState = &inputAssemblyStateCreateInfo,
            .pTessellationState = (descriptor.tessellation.has_value() ? &tessellationStateCreateInfo : nullptr),
            .pViewportState = nullptr,          // not used. viewport is supplied at draw time and not baked into the graphics pipeline
            .pRasterizationState = &rasterizationStateCreateInfo,
            .pMultisampleState = &multisampleStateCreateInfo,
            .pDepthStencilState = &depthStencilStateCreateInfo,
            .pColorBlendState = &colorBlendStateCreateInfo,
            .pDynamicState = &dynamicStateCreateInfo,
            .layout = vkPipelineLayout,
            .renderPass = nullptr,              // not used with dynamic rendering
            .subpass = 0,                       // not used with dynamic rendering
            .basePipelineHandle = nullptr,      // not used yet
            .basePipelineIndex = -1             // not used yet
        };

        const VkResult result = vkCreateGraphicsPipelines(context->device.vkDevice, context->device.vkPipelineCache, 1, &createInfo, nullptr, &resource.vkPipeline);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Graphics Pipeline with result ", result);
            return false;
        }

        return true;
    }

    GraphicsPipelineHandle ResourceManager::createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) noexcept
    {
        GraphicsPipelineResource resource{};
        GraphicsPipelineHandle handle{};

        if (createGraphicsPipelineResource(*this, m_pContext, resource, descriptor) == true)
        {
            resource.descriptor = descriptor;
            handle = m_graphicsPipelinePool.create(resource);

            m_shaderModuleReferenceMap.onGraphicsPipelineAdded(this, m_graphicsPipelinePool.get(handle));
        }
        else
        {
            logError("Error creating Vulkan Graphics Pipeline");
        }

        return handle;
    }

    GraphicsPipelineResource* ResourceManager::getGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        return m_graphicsPipelinePool.get(handle);
    }

    void ResourceManager::destroyGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        GraphicsPipelineResource* resource = m_graphicsPipelinePool.get(handle);

        if (resource != nullptr)
        {
            m_shaderModuleReferenceMap.onGraphicsPipelineDestroyed(this, resource);

            if (resource->vkPipeline != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(m_pContext->device.vkDevice, resource->vkPipeline, nullptr);
            }

            m_graphicsPipelinePool.destroy(handle);
        }
    }

    //--------------------------------------------------------------------------------------
    // Sampler
    //--------------------------------------------------------------------------------------

    SamplerHandle ResourceManager::createSampler(SamplerDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return SamplerHandle{};
    }

    SamplerResource* ResourceManager::getSampler(SamplerHandle handle) noexcept
    {
        return m_samplerPool.get(handle);
    }

    void ResourceManager::destroySampler(SamplerHandle handle) noexcept
    {
        if (m_samplerPool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // ShaderModule
    //--------------------------------------------------------------------------------------

    ShaderModuleHandle ResourceManager::getShaderModuleHandle(std::string const& resource) const noexcept
    {
        auto find = m_shaderModuleMap.find(resource);

        if (find != m_shaderModuleMap.end())
        {
            return find->second;
        }

        return {};
    }

    /// <summary>
    /// Creates the resource.
    /// This is split out because two paths need to be able to create a shader resource: createShaderModule and onShaderModuleReload.
    /// </summary>
    /// <param name="context"></param>
    /// <param name="resource"></param>
    /// <param name="descriptor"></param>
    /// <returns></returns>
    bool createShaderModuleResource(RendererContext* context, ShaderModuleResource& resource, ShaderModuleDescriptor const& descriptor) noexcept
    {
        if (descriptor.resource.empty())
        {
            logError("Empty resource name provided to Vulkan ShaderModule creation");
            return false;
        }

        if (descriptor.bytes.empty())
        {
            logError("Empty bytecode blob provided to Vulkan ShaderModule creation for resource '", descriptor.resource, "'");
            return false;
        }

        const VkShaderModuleCreateInfo shaderModuleInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = descriptor.bytes.size(),
            .pCode = reinterpret_cast<uint32_t const*>(descriptor.bytes.data())
        };

        // Compile the shader module
        const VkResult result = vkCreateShaderModule(context->device.vkDevice, &shaderModuleInfo, nullptr, &resource.vkShaderModule);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan ShaderModule with result ", result);
            return false;
        }

        // Reflect the SPIR-V for future pipeline binds
        auto reflection = reflectSPIRV(descriptor.bytes);

        if (reflection.has_value() && reflection->entryPoints.size() > 0)
        {
            resource.reflection = *reflection;
        }
        else
        {
            logError("Failed to reflect Vulkan Shader at '", descriptor.resource, "'");
            return false;
        }

        // Finish building the resource and then create the handle
        resource.spirvHash = hashArray(descriptor.bytes);
        resource.resource = descriptor.resource;
        resource.resourceId = StringId(descriptor.resource);

        return true;
    }

    ShaderModuleHandle ResourceManager::createShaderModule(ShaderModuleDescriptor const& descriptor) noexcept
    {
        ShaderModuleHandle handle = getShaderModuleHandle(descriptor.resource);

        if (handle.isValid())
        {
            // Already exists. Did it mean to do a reload?
            return handle;
        }

        ShaderModuleResource resource{};

        if (createShaderModuleResource(m_pContext, resource, descriptor))
        {
            handle = m_shaderModulePool.create(resource);

            // Record in the map
            m_shaderModuleReferenceMap.onShaderModuleAdded(m_shaderModulePool.get(handle));
            m_shaderModuleMap[resource.resource] = handle;
        }

        return handle;
    }

    ShaderModuleResource* ResourceManager::getShaderModule(ShaderModuleHandle handle) noexcept
    {
        return m_shaderModulePool.get(handle);
    }

    void ResourceManager::destroyShaderModule(ShaderModuleHandle handle) noexcept
    {
        ShaderModuleResource* resource = m_shaderModulePool.get(handle);

        if (resource != nullptr)
        {
            if (resource->vkShaderModule != VK_NULL_HANDLE)
            {
                vkDestroyShaderModule(m_pContext->device.vkDevice, resource->vkShaderModule, nullptr);
            }

            m_shaderModuleReferenceMap.onShaderModuleDestroyed(resource);
            m_shaderModulePool.destroy(handle);
            m_shaderModuleMap.erase(resource->resource);
        }
    }

    void ResourceManager::onShaderModuleReload(ShaderModuleDescriptor const& descriptor) noexcept
    {
        auto handle = getShaderModuleHandle(descriptor.resource);
        auto* resource = getShaderModule(handle);

        if (resource == nullptr)
        {
            // no active shader module matching the descriptor - nothing to reload
            return;
        }

        ShaderModuleResource reloadedResource{};

        if (createShaderModuleResource(m_pContext, reloadedResource, descriptor))
        {
            if (reloadedResource.spirvHash == resource->spirvHash)
            {
                // Identical, nothing needs to happen. Destroy the new duplicate module
                vkDestroyShaderModule(m_pContext->device.vkDevice, reloadedResource.vkShaderModule, nullptr);
            }
            else
            {
                // Recreate the shader module and any pipelines (both graphics and compute) that reference it.
                // As the renderer library deals only with handles, we simply recreate the underlying resources associated with the handles.

                VkShaderModule oldShaderModule = resource->vkShaderModule;

                resource->vkShaderModule = reloadedResource.vkShaderModule;
                resource->reflection = reloadedResource.reflection;
                resource->spirvHash = reloadedResource.spirvHash;

                std::vector<GraphicsPipelineResource*> affectedGraphicsPipelines;
                std::vector<ComputePipelineResource*> affectedComputePipelines;

                m_shaderModuleReferenceMap.getGraphicsPipelinesFor(resource, affectedGraphicsPipelines);
                m_shaderModuleReferenceMap.getComputePipelinesFor(resource, affectedComputePipelines);

                for (auto* graphicsPipelineResource : affectedGraphicsPipelines)
                {
                    VkPipeline oldGraphicsPipeline = graphicsPipelineResource->vkPipeline;

                    if (createGraphicsPipelineResource(*this, m_pContext, *graphicsPipelineResource, graphicsPipelineResource->descriptor))
                    {
                        vkDestroyPipeline(m_pContext->device.vkDevice, oldGraphicsPipeline, nullptr);
                    }
                    else
                    {
                        logError("Failed to recreate Vulkan Graphics Pipeline following reload of shader at '", descriptor.resource, "'");
                    }
                }

                for (auto* computePipelineResource : affectedComputePipelines)
                {
                    VkPipeline oldComputePipeline = computePipelineResource->vkPipeline;

                    if (createComputePipelineResource(*this, m_pContext, *computePipelineResource, computePipelineResource->descriptor))
                    {
                        vkDestroyPipeline(m_pContext->device.vkDevice, oldComputePipeline, nullptr);
                    }
                    else
                    {
                        logError("Failed to recreate Vulkan Compute Pipeline following reload of shader at '", descriptor.resource, "'");
                    }
                }

                vkDestroyShaderModule(m_pContext->device.vkDevice, oldShaderModule, nullptr);
            }
        }
        else
        {
            logError("Failed to recreate Vulkan Shader Module following reload of shader at '", descriptor.resource, "'");
        }
    }

    //--------------------------------------------------------------------------------------
    // Texture
    //--------------------------------------------------------------------------------------

    TextureHandle ResourceManager::createTexture(TextureDescriptor const& descriptor) noexcept
    {
        // ... todo ...
        return TextureHandle{};
    }

    TextureResource* ResourceManager::getTexture(TextureHandle handle) noexcept
    {
        return m_texturePool.get(handle);
    }

    void ResourceManager::destroyTexture(TextureHandle handle) noexcept
    {
        if (m_texturePool.destroy(handle))
        {
            // ... todo ...
        }
    }

    //--------------------------------------------------------------------------------------
    // Pipeline Layout
    //--------------------------------------------------------------------------------------

    VkDescriptorSetLayout ResourceManager::getOrCreateSetLayout(DescriptorSetLayoutDesc const& descriptorSetLayoutDesc) noexcept
    {
        return m_pipelineLayoutCache.getOrCreateSetLayout(descriptorSetLayoutDesc);
    }

    VkPipelineLayout ResourceManager::getOrCreatePipelineLayout(PipelineLayoutDescriptor const& pipelineLayoutDesc) noexcept
    {
        return m_pipelineLayoutCache.getOrCreatePipelineLayout(pipelineLayoutDesc);
    }
}