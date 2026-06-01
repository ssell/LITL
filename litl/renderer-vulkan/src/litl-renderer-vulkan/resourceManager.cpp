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
        // Caches
        m_pipelineLayoutCache.destroy();

        // Command Buffers
        std::vector<CommandBufferHandle> commandBufferHandles;
        m_commandBufferPool.getAllHandles(commandBufferHandles);

        for (auto commandBufferHandle : commandBufferHandles)
        {
            destroyCommandBuffer(commandBufferHandle);
        }

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

    ComputePipelineHandle ResourceManager::createComputePipeline(ComputePipelineDescriptor const& descriptor) noexcept
    {
        // ... todo ...
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
        }
    }

    //--------------------------------------------------------------------------------------
    // GraphicsPipeline
    //--------------------------------------------------------------------------------------

    void createPipelineShaderStageInfo(ShaderModuleResource* resource, GraphicsPipelineShaderDescriptor const& descriptor, std::array<VkPipelineShaderStageCreateInfo, 7>& stages, uint32_t& count) noexcept
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

        if ((static_cast<uint32_t>(entryPoint.value()->stage) & static_cast<uint32_t>(descriptor.stage)) == 0)
        {
            logWarning("Vulkan Graphics Pipeline creation of sahder with entry point of '", descriptor.entryPoint, "' failed due to declared shader stage mismatch. Declared = ", static_cast<uint32_t>(descriptor.stage), ", Expected = ", static_cast<uint32_t>(entryPoint.value()->stage));
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

    GraphicsPipelineHandle ResourceManager::createGraphicsPipeline(GraphicsPipelineDescriptor const& descriptor) noexcept
    {
        GraphicsPipelineResource resource{};

        // ---------------------------------------------------------------------------------
        // Shader Stages
        // ---------------------------------------------------------------------------------

        uint32_t shaderStageCount = 0;
        std::array<VkPipelineShaderStageCreateInfo, 7> shaderStages;

        createPipelineShaderStageInfo(getShaderModule(descriptor.vertex.handle), descriptor.vertex, shaderStages, shaderStageCount);
        createPipelineShaderStageInfo(getShaderModule(descriptor.fragment.handle), descriptor.fragment, shaderStages, shaderStageCount);
        createPipelineShaderStageInfo(getShaderModule(descriptor.geometry.handle), descriptor.geometry, shaderStages, shaderStageCount);
        createPipelineShaderStageInfo(getShaderModule(descriptor.tessellationEvaluation.handle), descriptor.tessellationEvaluation, shaderStages, shaderStageCount);
        createPipelineShaderStageInfo(getShaderModule(descriptor.tessellationControl.handle), descriptor.tessellationControl, shaderStages, shaderStageCount);
        createPipelineShaderStageInfo(getShaderModule(descriptor.mesh.handle), descriptor.mesh, shaderStages, shaderStageCount);

        // ---------------------------------------------------------------------------------
        // Input Assembly
        // ---------------------------------------------------------------------------------
        
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;

        const VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = createVertexInputStateCreateInfo(descriptor.vertexInput, vertexInputBindingDescriptions, vertexInputAttributeDescriptions);


        // ---------------------------------------------------------------------------------
        // Create the Graphics Pipeline
        // ---------------------------------------------------------------------------------

        const VkGraphicsPipelineCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = shaderStageCount,
            .pStages = shaderStages.data(),
            .pVertexInputState = &vertexInputCreateInfo
        };


        const VkResult result = vkCreateGraphicsPipelines(m_pContext->device.vkDevice, m_pContext->device.vkPipelineCache, 1, &createInfo, nullptr, &resource.vkPipeline);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Graphics Pipeline with result ", result);
            return {};
        }

        return m_graphicsPipelinePool.create(resource);
    }

    GraphicsPipelineResource* ResourceManager::getGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        return m_graphicsPipelinePool.get(handle);
    }

    void ResourceManager::destroyGraphicsPipeline(GraphicsPipelineHandle handle) noexcept
    {
        if (m_graphicsPipelinePool.destroy(handle))
        {
            // ... todo ...
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

    ShaderModuleHandle ResourceManager::createShaderModule(ShaderModuleDescriptor const& descriptor) noexcept
    {
        if (descriptor.resource.empty())
        {
            logError("Empty resource name provided to Vulkan ShaderModule creation");
            return {};
        }

        if (descriptor.bytes.empty())
        {
            logError("Empty bytecode blob provided to Vulkan ShaderModule creation for resource '", descriptor.resource, "'");
            return {};
        }

        ShaderModuleHandle handle = getShaderModuleHandle(descriptor.resource);

        if (handle.isValid())
        {
            return handle;
        }

        ShaderModuleResource resource{};

        const VkShaderModuleCreateInfo shaderModuleInfo{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = descriptor.bytes.size(),
            .pCode = reinterpret_cast<uint32_t const*>(descriptor.bytes.data())
        };

        // Compile the shader module
        const VkResult result = vkCreateShaderModule(m_pContext->device.vkDevice, &shaderModuleInfo, nullptr, &resource.vkShaderModule);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan ShaderModule with result ", result);
            return {};
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
            return {};
        }

        // Finish building the resource and then create the handle
        resource.spirvHash = hashArray(descriptor.bytes);
        resource.resource = descriptor.resource;

        handle = m_shaderModulePool.create(resource);

        // Record in the map
        m_shaderModuleMap[resource.resource] = handle;

        return handle;
    }

    ShaderModuleResource* ResourceManager::getShaderModule(ShaderModuleHandle handle) noexcept
    {
        return m_shaderModulePool.get(handle);
    }

    void ResourceManager::destroyShaderModule(ShaderModuleHandle handle) noexcept
    {
        ShaderModuleResource* shaderModule = m_shaderModulePool.get(handle);

        if (shaderModule != nullptr)
        {
            if (shaderModule->vkShaderModule != VK_NULL_HANDLE)
            {
                vkDestroyShaderModule(m_pContext->device.vkDevice, shaderModule->vkShaderModule, nullptr);
            }

            m_shaderModulePool.destroy(handle);
            m_shaderModuleMap.erase(shaderModule->resource);
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