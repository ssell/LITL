#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-core/assert.hpp"

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // Internal Utilities
    // -------------------------------------------------------------------------------------

    CommandBufferResource* unwrapCommandBuffer(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* vulkanContext = unwrap(context);

        if (vulkanContext != nullptr)
        {
            return vulkanContext->resources.getCommandBuffer(handle);
        }

        return nullptr;
    }

    bool isValid(CommandBufferResource* resource) noexcept
    {
        return (resource != nullptr) && (resource->vkCommandBuffer != VK_NULL_HANDLE);
    }

    struct BoundPipeline
    {
        GraphicsPipelineResource* graphics = nullptr;
        ComputePipelineResource* compute = nullptr;

        [[nodiscard]] constexpr bool isValid() const noexcept
        {
            return (graphics != nullptr) || (compute != nullptr);
        }

        [[nodiscard]] VkPipelineLayout getPipelineLayout() const noexcept
        {
            if (graphics != nullptr)
            {
                return graphics->pipeline.vkPipelineLayout;
            }
            else if (compute != nullptr)
            {
                return compute->pipeline.vkPipelineLayout;
            }

            return VK_NULL_HANDLE;
        }

        [[nodiscard]] PipelineResourceBinding const* getResourceBinding(StringId id) const noexcept
        {
            if (graphics != nullptr)
            {
                return graphics->pipeline.resourceMap.getResourceBinding(id);
            }
            else if (compute != nullptr)
            {
                return compute->pipeline.resourceMap.getResourceBinding(id);
            }

            return nullptr;
        }

        [[nodiscard]] VkDescriptorSetLayout getSetLayout(uint32_t set) const noexcept
        {
            if (graphics != nullptr)
            {
                if (graphics->pipeline.setLayouts.size() > set)
                {
                    return graphics->pipeline.setLayouts[set];
                }
            }
            else if (compute != nullptr)
            {
                if (compute->pipeline.setLayouts.size() > set)
                {
                    return compute->pipeline.setLayouts[set];
                }
            }

            return VK_NULL_HANDLE;
        }
    };

    BoundPipeline getBoundPipeline(RendererContext* vulkanContext, CommandBufferResource* commandBuffer, bool isGraphicsPipeline)
    {
        if (isGraphicsPipeline)
        {
            return BoundPipeline { .graphics = vulkanContext->resources.getGraphicsPipeline(commandBuffer->boundGraphicsPipeline) };
        }
        else
        {
            return BoundPipeline{ .compute = vulkanContext->resources.getComputePipeline(commandBuffer->boundComputePipeline) };
        }
    }

    // -------------------------------------------------------------------------------------
    // General
    // -------------------------------------------------------------------------------------

    CommandBufferHandle cmdBeginFrame(litl::RendererContext* context) noexcept
    {
        auto commandBufferHandle = unwrap(context)->getCurrFrameSyncInfo().commandBuffer;

        if (!cmdBegin(context, commandBufferHandle))
        {
            return {};  // return invalid handle (.isValid() == false)
        }

        return commandBufferHandle;
    }

    bool cmdBegin(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return false;
        }

        VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        if (commandBuffer->isTransient)
        {
            info.flags |= VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        }
        else
        {
            commandBuffer->descriptorSetChanges.reset();
        }

        return (vkBeginCommandBuffer(commandBuffer->vkCommandBuffer, &info) == VK_SUCCESS);
    }

    bool cmdEnd(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return false;
        }

        if (commandBuffer->boundGraphicsPipeline.isValid())
        {
            commandBuffer->boundGraphicsPipeline = {};
        }

        return vkEndCommandBuffer(commandBuffer->vkCommandBuffer) == VK_SUCCESS;
    }

    [[nodiscard]] std::optional<VkRenderingAttachmentInfo> createCmdBeginRenderColorAttachment(BeginRenderCommand const& command, RendererContext* vulkanContext) noexcept
    {
        if (command.colorAttachmentType == RenderAttachmentType::None)
        {
            return std::nullopt;
        }

        const ColorAttachmentDescriptor cmdColorAttachment = command.colorAttachment.value_or({});

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = VK_NULL_HANDLE;
        colorAttachment.imageLayout = toVkImageLayout(ImageLayoutType::Color);
        colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        colorAttachment.clearValue = VkClearValue{ .color = toVkClearColorValue(cmdColorAttachment.clearColor) };

        if (command.colorAttachmentType == RenderAttachmentType::Supplied)
        {
            TextureResource const* colorTexture = vulkanContext->resources.getTexture(cmdColorAttachment.colorTexture);
            VkExtent2D textureExtents{ .width = 1u, .height = 1u };

            if (colorTexture == nullptr)
            {
                return std::nullopt;
            }

            colorAttachment.imageView = colorTexture->vkImageView;
            colorAttachment.loadOp = toVkAttachmentLoadOp(cmdColorAttachment.loadOp);
            colorAttachment.storeOp = toVkAttachmentStoreOp(cmdColorAttachment.storeOp);
            vulkanContext->drawInfo.targetTextureSize = VkExtent2D{ .width = colorTexture->descriptor.width, .height = colorTexture->descriptor.height };
        }
        else if (command.colorAttachmentType == RenderAttachmentType::Swapchain)
        {
            colorAttachment.imageView = vulkanContext->swapChain.vkSwapChainImageViews[vulkanContext->swapChain.swapChainImageIndex];
            colorAttachment.loadOp = toVkAttachmentLoadOp(cmdColorAttachment.loadOp);
            colorAttachment.storeOp = toVkAttachmentStoreOp(cmdColorAttachment.storeOp);
            vulkanContext->drawInfo.targetTextureSize = vulkanContext->swapChain.vkSwapChainExtent;
        }


        return colorAttachment;
    }

    [[nodiscard]] std::optional<VkRenderingAttachmentInfo> createCmdBeginRenderDepthAttachment(BeginRenderCommand const& command, RendererContext* vulkanContext, bool& hasStencil) noexcept
    {
        hasStencil = false;
        vulkanContext->drawInfo.depthFormat = VK_FORMAT_UNDEFINED;
        vulkanContext->drawInfo.stencilFormat = VK_FORMAT_UNDEFINED;

        if (command.depthAttachmentType == RenderAttachmentType::None)
        {
            return std::nullopt;
        }

        const DepthAttachmentDescriptor cmdDepthAttachment = command.depthAttachment.value_or({});

        TextureResource const* depthTexture = !cmdDepthAttachment.depthTexture.isValid() ?
            vulkanContext->resources.getTexture(vulkanContext->getCurrFrameDepthTexture()) :        // No depth texture supplied, use current frame depth texture
            vulkanContext->resources.getTexture(cmdDepthAttachment.depthTexture);                     // Use provided depth texture.

        if (depthTexture == nullptr)
        {
            return std::nullopt;
        }

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageLayout = toVkImageLayout(ImageLayoutType::DepthStencil);
        depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;

        const auto dataFormat = fromVkFormat(depthTexture->vkFormat);

        if (dataFormatHasDepth(dataFormat))
        {
            vulkanContext->drawInfo.depthFormat = depthTexture->vkFormat;

            if (dataFormatHasStencil(dataFormat))
            {
                vulkanContext->drawInfo.stencilFormat = depthTexture->vkFormat;
                hasStencil = true;
            }

            depthAttachment.imageView  = depthTexture->vkImageView;
            depthAttachment.loadOp     = toVkAttachmentLoadOp(cmdDepthAttachment.loadOp);
            depthAttachment.storeOp    = toVkAttachmentStoreOp(cmdDepthAttachment.storeOp);
            depthAttachment.clearValue = VkClearValue{
                .depthStencil = VkClearDepthStencilValue{
                    .depth = cmdDepthAttachment.clearDepth,
                    .stencil = cmdDepthAttachment.clearStencil
                }
            };

            if (command.colorAttachmentType == RenderAttachmentType::None)
            {
                vulkanContext->drawInfo.targetTextureSize = VkExtent2D{ .width = depthTexture->descriptor.width, .height = depthTexture->descriptor.height };
            }
        }

        return depthAttachment;
    }

    bool cmdBeginRender(litl::RendererContext* context, CommandBufferHandle handle, BeginRenderCommand const& command) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return false;
        }

        // ---------------------------------------------------------------------------------
        // Get Attachments
        // ---------------------------------------------------------------------------------

        bool hasStencil{ false };

        const auto colorAttachment = createCmdBeginRenderColorAttachment(command, vulkanContext);
        const auto depthAttachment = createCmdBeginRenderDepthAttachment(command, vulkanContext, hasStencil);

        const auto colorAttachmentCount = (colorAttachment.has_value() ? 1u : 0u);
        const auto* colorAttachmentPtr = (colorAttachment.has_value() ? &colorAttachment.value() : nullptr);
        const auto* depthAttachmentPtr = (depthAttachment.has_value() ? &depthAttachment.value() : nullptr);

        if ((command.colorAttachmentType != RenderAttachmentType::None) && !colorAttachment.has_value())
        {
            logError("Failed to retrieve color attachment for cmdBeginRender.");
            return false;
        }

        if ((command.depthAttachmentType != RenderAttachmentType::None) && !depthAttachment.has_value())
        {
            logError("Failed to retrieve depth attachment for cmdBeginRender.");
            return false;
        }

        // ---------------------------------------------------------------------------------
        // vkCmdBeginRendering
        // ---------------------------------------------------------------------------------

        const VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .flags = 0,
            .renderArea = VkRect2D {            // both command.area.offset and command.area.extents are vectors in normalized texture space on the range of [0, 1]
                .offset = VkOffset2D {
                    .x = static_cast<int32_t>(vulkanContext->drawInfo.targetTextureSize.width * command.area.offset.x()),
                    .y = static_cast<int32_t>(vulkanContext->drawInfo.targetTextureSize.height * command.area.offset.y())
                },
                .extent = (command.colorAttachmentType == RenderAttachmentType::Swapchain) ?
                        vulkanContext->swapChain.vkSwapChainExtent :
                        VkExtent2D {
                            .width = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.width * command.area.extents.x()),
                            .height = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.height * command.area.extents.y())
                        }
            },
            .layerCount = command.layerCount,
            .viewMask = command.viewMask,                       // the graphics pipeline view mask must match this
            .colorAttachmentCount = colorAttachmentCount,
            .pColorAttachments = colorAttachmentPtr,
            .pDepthAttachment = depthAttachmentPtr,
            .pStencilAttachment = (hasStencil ? depthAttachmentPtr : nullptr)
        };

        vkCmdBeginRendering(commandBuffer->vkCommandBuffer, &renderingInfo);

        // ---------------------------------------------------------------------------------
        // Bind the Global Texture Table
        // ---------------------------------------------------------------------------------

        const VkDescriptorSet textureTableDescriptorSet = vulkanContext->textureTable.getDescriptorSet();

        vkCmdBindDescriptorSets(
            commandBuffer->vkCommandBuffer,
            VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
            vulkanContext->textureTable.getPipelineLayout(),
            static_cast<uint32_t>(DescriptorSetIndex::PerFrame),
            1u,
            &textureTableDescriptorSet,
            0,
            nullptr);

        // todo update for Compute pipeline if/when compute shaders ever need to sample from the global texture table

        return true;
    }

    void cmdEndRender(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        vkCmdEndRendering(commandBuffer->vkCommandBuffer);
    }

    void cmdPipelineBarrier(litl::RendererContext* context, CommandBufferHandle handle, PipelineBarrierCommand const& command) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        VkImage vkImage = VK_NULL_HANDLE;
        VkImageSubresourceRange subresourceRange{};

        if (command.texture.isValid())
        {
            auto* texture = vulkanContext->resources.getTexture(command.texture);

            if (texture == nullptr)
            {
                logError("Failed to issue cmdPipelineBarrier - failed to retrieve texture specified in command.");
                return;
            }

            vkImage = texture->vkImage;
            subresourceRange = texture->vkImageSubresourceRange;
        }
        else
        {
            // Default to the swapchain texture if no texture is specified.
            if ((command.toLayout == ImageLayoutType::DepthStencil) || (command.toLayout == ImageLayoutType::DepthStencilReadOnly))
            {
                auto* depthTexture = vulkanContext->resources.getTexture(vulkanContext->getCurrFrameSyncInfo().depthTexture);

                if (depthTexture == nullptr)
                {
                    logError("Failed to issue cmdPipelineBarrier - failed to retrieve swapchain depth texture.");
                    return;
                }

                vkImage = depthTexture->vkImage;
                subresourceRange = depthTexture->vkImageSubresourceRange;
            }
            else
            {
                vkImage = vulkanContext->swapChain.vkSwapChainImages[vulkanContext->swapChain.swapChainImageIndex];
                subresourceRange = VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1
                };
            }
        }

        const VkImageMemoryBarrier2 barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = toVkPipelineStageFlag(command.sourceStage),
            .srcAccessMask = toVkAccessFlag(command.sourceAccess),
            .dstStageMask = toVkPipelineStageFlag(command.destStage),
            .dstAccessMask = toVkAccessFlag(command.destAccess),
            .oldLayout = toVkImageLayout(command.fromLayout),
            .newLayout = toVkImageLayout(command.toLayout),
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = vkImage,
            .subresourceRange = subresourceRange
        };

        const VkDependencyInfo info{
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .dependencyFlags = {},
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        vkCmdPipelineBarrier2(commandBuffer->vkCommandBuffer, &info);
    }

    void cmdClearImage(litl::RendererContext* context, CommandBufferHandle handle, ClearImageCommand const& command) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        const VkClearColorValue clearColor = {
            {
                command.clearColor.r(),
                command.clearColor.g(),
                command.clearColor.b(),
                command.clearColor.a()
            }
        };

        const VkImageSubresourceRange range{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        TextureResource* targetTexture = vulkanContext->resources.getTexture(command.image);
        VkImage targetImage = VK_NULL_HANDLE;

        if (targetTexture != nullptr)
        {
            targetImage = targetTexture->vkImage;
        }
        else
        {
            // clear the current swap chain image
            targetImage = vulkanContext->swapChain.vkSwapChainImages[vulkanContext->swapChain.swapChainImageIndex];
        }

        vkCmdClearColorImage(
            commandBuffer->vkCommandBuffer,
            targetImage,
            toVkImageLayout(command.destLayout),
            &clearColor,
            1,
            &range);
    }

    void cmdSetViewportAndScissor(litl::RendererContext* context, CommandBufferHandle handle, SetViewportAndScissorCommand const& command) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        if (command.setViewport.has_value())
        {
            // Note: incoming viewport regions are normalized so that the command caller does not need to track texture dimensions.
            const VkViewport viewport{
                .x = vulkanContext->drawInfo.targetTextureSize.width * command.setViewport->region.offset.x(),
                .y = vulkanContext->drawInfo.targetTextureSize.height * command.setViewport->region.offset.y(),
                .width = vulkanContext->drawInfo.targetTextureSize.width * command.setViewport->region.extents.x(),
                .height = vulkanContext->drawInfo.targetTextureSize.height * command.setViewport->region.extents.y(),
                .minDepth = command.setViewport->minDepth,
                .maxDepth = command.setViewport->maxDepth
            };

            vkCmdSetViewportWithCount(commandBuffer->vkCommandBuffer, 1, &viewport);
        }

        if (command.setScissor.has_value())
        {
            // Note: incoming scissor regions are normalized so that the command caller does not need to track texture dimensions.
            const VkRect2D scissorRegion{
                .offset = VkOffset2D {
                    .x = static_cast<int32_t>(vulkanContext->drawInfo.targetTextureSize.width * command.setScissor->region.offset.x()),
                    .y = static_cast<int32_t>(vulkanContext->drawInfo.targetTextureSize.height * command.setScissor->region.offset.y())
                },
                .extent = VkExtent2D {
                    .width = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.width * command.setScissor->region.extents.x()),
                    .height = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.height * command.setScissor->region.extents.y())
                }
            };

            vkCmdSetScissorWithCount(commandBuffer->vkCommandBuffer, 1, &scissorRegion);
        }
    }

    void cmdBindGraphicsPipeline(litl::RendererContext* context, CommandBufferHandle handle, GraphicsPipelineHandle graphicsPipelineHandle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        if (commandBuffer->boundGraphicsPipeline == graphicsPipelineHandle)
        {
            return;
        }

        GraphicsPipelineResource* prevPipeline = vulkanContext->resources.getGraphicsPipeline(commandBuffer->boundGraphicsPipeline);
        GraphicsPipelineResource* currPipeline = vulkanContext->resources.getGraphicsPipeline(graphicsPipelineHandle);

        LITL_ASSERT_MSG((currPipeline != nullptr), "Invalid GraphicsPipelineHandle provided to cmdBindGraphicsPipeline", );

        vkCmdBindPipeline(commandBuffer->vkCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, currPipeline->pipeline.vkPipeline);

        commandBuffer->boundGraphicsPipeline = graphicsPipelineHandle;
        commandBuffer->descriptorSetChanges.onPipelineLayoutChange((prevPipeline != nullptr ? &prevPipeline->pipeline : nullptr), currPipeline->pipeline);
    }

    RendererResult cmdPushConstants(litl::RendererContext* context, CommandBufferHandle handle, ShaderStage shaderStage, std::span<std::byte const> data) noexcept
    {
        if (data.size() > RendererConstants::MaxPushConstantSize)
        {
            return RendererResult::InvalidPushConstantSize;
        }

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* graphicsPipelineResource = vulkanContext->resources.getGraphicsPipeline(commandBuffer->boundGraphicsPipeline);

        if (graphicsPipelineResource == nullptr)     /* todo check compute pipeline */
        {
            return RendererResult::NoBoundGraphicsPipeline;
        }

        vkCmdPushConstants(
            commandBuffer->vkCommandBuffer,
            graphicsPipelineResource->pipeline.vkPipelineLayout,
            toVkShaderStageFlags(shaderStage),
            0,
            static_cast<uint32_t>(data.size()),
            data.data());

        return RendererResult::Success;
    }

    RendererResult cmdBindVertexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, uint64_t offset, uint32_t firstBinding) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* bufferResource = vulkanContext->resources.getBuffer(bufferHandle);

        if ((bufferResource == nullptr) || (bufferResource->vkBuffer == nullptr))
        {
            return RendererResult::InvalidBufferHandle;
        }

        vkCmdBindVertexBuffers(
            commandBuffer->vkCommandBuffer, 
            firstBinding, 
            1, 
            &bufferResource->vkBuffer, &offset);

        return RendererResult::Success;
    }

    RendererResult cmdBindVertexBuffers(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle* bufferHandles, uint64_t* offsets, uint32_t count, uint32_t firstBinding) noexcept
    {
        if (bufferHandles == nullptr)
        {
            return RendererResult::NullSource;
        }

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        std::vector<VkBuffer> buffers;
        buffers.reserve(count);

        for (auto i = 0; i < count; ++i)
        {
            auto* resource = vulkanContext->resources.getBuffer(bufferHandles[i]);

            if ((resource == nullptr) || (resource->vkBuffer == VK_NULL_HANDLE))
            {
                return RendererResult::InvalidBufferHandle;
            }

            buffers.push_back(resource->vkBuffer);
        }

        vkCmdBindVertexBuffers(
            commandBuffer->vkCommandBuffer, 
            firstBinding,
            count, 
            buffers.data(), 
            offsets);

        return RendererResult::Success;
    }

    RendererResult cmdBindIndexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, IndexType indexType) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* bufferResource = vulkanContext->resources.getBuffer(bufferHandle);

        if ((bufferResource == nullptr) || (bufferResource->vkBuffer == nullptr))
        {
            return RendererResult::InvalidBufferHandle;
        }

        vkCmdBindIndexBuffer(
            commandBuffer->vkCommandBuffer, 
            bufferResource->vkBuffer, 
            0, 
            toVkIndexType(indexType));

        return RendererResult::Success;
    }

    RendererResult cmdBindBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, StringId key, uint64_t offset, uint64_t range, bool isGraphics) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto boundPipeline = getBoundPipeline(vulkanContext, commandBuffer, isGraphics);

        if (!boundPipeline.isValid())
        {
            return (isGraphics ? RendererResult::NoBoundGraphicsPipeline : RendererResult::NoBoundComputePipeline);
        }

        auto* bufferResource = vulkanContext->resources.getBuffer(bufferHandle);

        if ((bufferResource == nullptr) || (bufferResource->vkBuffer == VK_NULL_HANDLE))
        {
            return RendererResult::InvalidBufferHandle;
        }

        auto* bindingResource = boundPipeline.getResourceBinding(key);

        if (bindingResource == nullptr)
        {
            return RendererResult::InvalidPipelineResourceKey;
        }

        // Defer the change to the next draw command
        commandBuffer->descriptorSetChanges.addChange(
            bindingResource->binding,
            bindingResource->set,
            toVkDescriptorType(bindingResource->type),
            VkDescriptorBufferInfo{
                .buffer = bufferResource->vkBuffer,
                .offset = offset,
                .range = range
            });

        return RendererResult::Success;
    }

    RendererResult cmdBufferUpload(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, std::span<std::byte const> source, BufferHandle destBufferHandle, uint64_t sourceOffset, uint64_t destOffset) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* destBuffer = vulkanContext->resources.getBuffer(destBufferHandle);

        if (destBuffer == nullptr)
        {
            return RendererResult::InvalidBufferHandle;
        }

        auto& frameSync = vulkanContext->getCurrFrameSyncInfo();

        // Source -> Staging
        auto stagingIndex = frameSync.stagingBufferArena->copyIntoStaging(
            source, 
            sourceOffset,
            source.size_bytes() - sourceOffset,
            4ull);

        if (!stagingIndex.has_value())
        {
            return RendererResult::MemoryCopyFailed;
        }

        // Staging -> Destination
        const bool result = frameSync.stagingBufferArena->copyIntoDestination(
            commandBuffer, 
            stagingIndex.value(),
            destBuffer, 
            destOffset);

        if (!result)
        {
            return RendererResult::MemoryCopyFailed;
        }

        return RendererResult::Success;
    }

    RendererResult cmdBufferFlush(litl::RendererContext* context, CommandBufferHandle commandBufferHandle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        vulkanContext->getCurrFrameSyncInfo().stagingBufferArena->flushBuffers(commandBuffer);

        return RendererResult::Success;
    }

    RendererResult cmdBindTexture(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, TextureResourceHandle textureHandle, StringId textureId, bool isGraphics) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto boundPipeline = getBoundPipeline(vulkanContext, commandBuffer, isGraphics);

        if (!boundPipeline.isValid())
        {
            return (isGraphics ? RendererResult::NoBoundGraphicsPipeline : RendererResult::NoBoundComputePipeline);
        }

        auto* textureResource = vulkanContext->resources.getTexture(textureHandle);

        if ((textureResource == nullptr) || (textureResource->vkImage == VK_NULL_HANDLE))
        {
            return RendererResult::InvalidTextureHandle;
        }

        auto* bindingResource = boundPipeline.getResourceBinding(textureId);

        if (bindingResource == nullptr)
        {
            return RendererResult::InvalidPipelineResourceKey;
        }

        // Defer the change to the next draw command
        commandBuffer->descriptorSetChanges.addChange(
            bindingResource->binding,
            bindingResource->set,
            toVkDescriptorType(bindingResource->type),
            VkDescriptorImageInfo{
                .sampler = VK_NULL_HANDLE,
                .imageView = (textureResource->vkSampledImageView != VK_NULL_HANDLE) ? textureResource->vkSampledImageView : textureResource->vkImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            });

        return RendererResult::Success;
    }

    RendererResult cmdBindSampler(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, SamplerHandle samplerHandle, StringId samplerId, bool isGraphics) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto boundPipeline = getBoundPipeline(vulkanContext, commandBuffer, isGraphics);

        if (!boundPipeline.isValid())
        {
            return (isGraphics ? RendererResult::NoBoundGraphicsPipeline : RendererResult::NoBoundComputePipeline);
        }

        auto* samplerResource = vulkanContext->resources.getSampler(samplerHandle);

        if ((samplerResource == nullptr) || (samplerResource->vkSampler == VK_NULL_HANDLE))
        {
            return RendererResult::InvalidSamplerHandle;
        }

        auto* bindingResource = boundPipeline.getResourceBinding(samplerId);

        if (bindingResource == nullptr)
        {
            return RendererResult::InvalidPipelineResourceKey;
        }

        // Defer the change to the next draw command
        commandBuffer->descriptorSetChanges.addChange(
            bindingResource->binding,
            bindingResource->set,
            toVkDescriptorType(bindingResource->type),
            VkDescriptorImageInfo{
                .sampler = samplerResource->vkSampler,
                .imageView = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_UNDEFINED
            });

        return RendererResult::Success;
    }

    RendererResult cmdTextureUpload(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, std::span<std::byte const> source, std::span<TextureUploadRegion const> regions, TextureResourceHandle destTextureHandle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* destTexture = vulkanContext->resources.getTexture(destTextureHandle);

        if (destTexture == nullptr)
        {
            return RendererResult::InvalidTextureHandle;
        }

        std::vector<TextureUploadRegion> defaultRegions;

        if (regions.empty())
        {
            buildTightlyPackedUploadRegions(destTexture->descriptor, defaultRegions);
            regions = defaultRegions;
        }

        std::vector<StagingTextureIndex> stagingIndices;
        stagingIndices.reserve(regions.size());

        auto& frameSync = vulkanContext->getCurrFrameSyncInfo();
        const uint64_t copyAlignment = dataFormatBufferCopyAlignment(destTexture->descriptor.format);

        for (auto& region : regions)
        {
            // Source -> Staging
            auto stagingIndex = frameSync.stagingTextureArena->copyIntoStaging(
                source, 
                region.sourceOffset, 
                imageLevelBytes(destTexture->descriptor.format, region.width, region.height, region.depth), 
                copyAlignment);

            if (!stagingIndex.has_value())
            {
                return RendererResult::MemoryCopyFailed;
            }

            stagingIndices.push_back(stagingIndex.value());
        }

        // Staging -> Destination
        const bool result = frameSync.stagingTextureArena->copyIntoDestination(
            commandBuffer,
            stagingIndices,
            regions,
            destTexture);

        if (!result)
        {
            return RendererResult::MemoryCopyFailed;
        }

        return RendererResult::Success;
    }

    void cmdDraw(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        GraphicsPipelineResource* graphicsPipeline = vulkanContext->resources.getGraphicsPipeline(commandBuffer->boundGraphicsPipeline);

        LITL_ASSERT_MSG((graphicsPipeline != nullptr), "cmdDraw called without a bound Graphics Pipeline", );

        commandBuffer->descriptorSetChanges.flushChanges(
            *vulkanContext,
            commandBuffer->vkCommandBuffer,
            graphicsPipeline->pipeline,
            true);

        vkCmdDraw(
            commandBuffer->vkCommandBuffer,
            vertexCount,
            instanceCount,
            firstVertex,
            firstInstance);
    }

    void cmdDrawIndexed(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        GraphicsPipelineResource* graphicsPipeline = vulkanContext->resources.getGraphicsPipeline(commandBuffer->boundGraphicsPipeline);

        LITL_ASSERT_MSG((graphicsPipeline != nullptr), "cmdDrawIndexed called without a bound Graphics Pipeline", );

        commandBuffer->descriptorSetChanges.flushChanges(
            *vulkanContext,
            commandBuffer->vkCommandBuffer,
            graphicsPipeline->pipeline,
            true);

        vkCmdDrawIndexed(
            commandBuffer->vkCommandBuffer,
            indexCount,
            instanceCount,
            firstIndex,
            vertexOffset,
            firstInstance);
    }
}