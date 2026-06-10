#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/conversions.hpp"
#include "litl-core/assert.hpp"

namespace litl::vulkan
{
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

        const VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };

        return vkBeginCommandBuffer(commandBuffer->vkCommandBuffer, &info) == VK_SUCCESS;
    }

    bool cmdEnd(litl::RendererContext* context, CommandBufferHandle handle) noexcept
    {
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return false;
        }

        return vkEndCommandBuffer(commandBuffer->vkCommandBuffer) == VK_SUCCESS;
    }

    void cmdBeginRender(litl::RendererContext* context, CommandBufferHandle handle, BeginRenderCommand const& command) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        // ---------------------------------------------------------------------------------
        // Color Texture Attachment
        // ---------------------------------------------------------------------------------

        const bool isSwapChain = !command.color.colorTexture.isValid();

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = VK_NULL_HANDLE;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
        colorAttachment.clearValue = VkClearValue{ .color = VkClearColorValue{ command.color.clearColor.r(), command.color.clearColor.g(), command.color.clearColor.b(), command.color.clearColor.a() } };

        if (isSwapChain)
        {
            // Swapchain color attachment
            colorAttachment.imageView = vulkanContext->swapChain.vkSwapChainImageViews[vulkanContext->swapChain.swapChainImageIndex];
            colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
            vulkanContext->drawInfo.targetTextureSize = vulkanContext->swapChain.vkSwapChainExtent;
        }
        else
        {
            // Custom color attachment info
            TextureResource* colorTexture = vulkanContext->resources.getTexture(command.color.colorTexture);

            if (colorTexture != nullptr)
            {
                colorAttachment.imageView = colorTexture->vkImageView;
            }

            colorAttachment.loadOp = toVkAttachmentLoadOp(command.color.loadOp);
            colorAttachment.storeOp = toVkAttachmentStoreOp(command.color.storeOp);
            vulkanContext->drawInfo.targetTextureSize = colorTexture->extent;
        }

        // ---------------------------------------------------------------------------------
        // Depth Texture Attachment
        // ---------------------------------------------------------------------------------

        VkRenderingAttachmentInfo depthAttachment{};

        if (command.depth.has_value())
        {
            VkImageView depthTextureView = VK_NULL_HANDLE;
            TextureResource* depthTexture = vulkanContext->resources.getTexture((*command.depth).depthTexture);

            if (depthTexture != nullptr)
            {
                depthTextureView = depthTexture->vkImageView;
                vulkanContext->drawInfo.depthFormat = depthTexture->vkFormat;
            }
            else
            {
                vulkanContext->drawInfo.depthFormat = VkFormat::VK_FORMAT_UNDEFINED;
            }

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = depthTextureView;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.loadOp = toVkAttachmentLoadOp((*command.depth).loadOp);
            depthAttachment.storeOp = toVkAttachmentStoreOp((*command.depth).storeOp);
            depthAttachment.clearValue = VkClearValue{
                .depthStencil = VkClearDepthStencilValue{
                    .depth = (*command.depth).clearDepth,
                    .stencil = (*command.depth).clearStencil
                }
            };
        }

        // ---------------------------------------------------------------------------------
        // Stencil Texture Attachment
        // ---------------------------------------------------------------------------------

        // ... todo ...

        // ---------------------------------------------------------------------------------
        // Begin Render
        // ---------------------------------------------------------------------------------
        
        VkRenderingInfo renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .flags = 0,
            .renderArea = { 
                .offset = { 
                    .x = static_cast<int32_t>(command.area.offset.x()), 
                    .y = static_cast<int32_t>(command.area.offset.y()) 
                },
                .extent = { 
                    .width  = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.width * command.area.extents.x()),
                    .height = static_cast<uint32_t>(vulkanContext->drawInfo.targetTextureSize.height * command.area.extents.y())
                }
            },
            .layerCount = command.layerCount,
            .viewMask = command.viewMask,           // the graphics pipeline view mask must match this
            .colorAttachmentCount = 1u,
            .pColorAttachments = &colorAttachment,
            .pDepthAttachment = command.depth.has_value() ? &depthAttachment : nullptr,
            .pStencilAttachment = nullptr
        };

        if (isSwapChain)
        {
            renderingInfo.renderArea.extent = vulkanContext->swapChain.vkSwapChainExtent;
        }

        vkCmdBeginRendering(commandBuffer->vkCommandBuffer, &renderingInfo);
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

        if (command.texture.isValid())
        {
            auto* texture = vulkanContext->resources.getTexture(command.texture);

            if (texture != nullptr)
            {
                vkImage = texture->vkImage;
            }
        }
        else
        {
            // Default to the swapchain if no texture is specified.
            vkImage = vulkanContext->swapChain.vkSwapChainImages[vulkanContext->swapChain.swapChainImageIndex];
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
            .subresourceRange = VkImageSubresourceRange {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
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
                .x = command.setViewport->region.offset.x(),
                .y = command.setViewport->region.offset.y(),
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
                    .x = static_cast<int32_t>(command.setScissor->region.offset.x()),
                    .y = static_cast<int32_t>(command.setScissor->region.offset.y())
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
        LITL_ASSERT_MSG(graphicsPipelineHandle.isValid(), "Invalid GraphicsPipelineHandle provided to cmdBindGraphicsPipeline", );

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, handle);

        if (!isValid(commandBuffer))
        {
            return;
        }

        GraphicsPipelineResource* graphicsPipeline = vulkanContext->resources.getGraphicsPipeline(graphicsPipelineHandle);

        if (graphicsPipeline != nullptr)
        {
            vkCmdBindPipeline(commandBuffer->vkCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline->vkPipeline);
        }
    }

    RendererResult cmdBindVertexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, uint64_t offset) noexcept
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

        vkCmdBindVertexBuffers(commandBuffer->vkCommandBuffer, 0, 1, &bufferResource->vkBuffer, &offset);

        return RendererResult::Success;
    }

    RendererResult cmdBindVertexBuffers(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle* bufferHandles, uint64_t* offsets, uint32_t count) noexcept
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

        vkCmdBindVertexBuffers(commandBuffer->vkCommandBuffer, 0, count, buffers.data(), offsets);
    }

    RendererResult cmdBindIndexBuffer(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle) noexcept
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

        vkCmdBindIndexBuffer(commandBuffer->vkCommandBuffer, bufferResource->vkBuffer, 0, VK_INDEX_TYPE_UINT32);

        return RendererResult::Success;
    }

    RendererResult cmdBufferUpload(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, std::span<std::byte const> source, BufferHandle destBufferHandle) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        // ... 

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

        // ... 

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

        vkCmdDraw(commandBuffer->vkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
    }
}