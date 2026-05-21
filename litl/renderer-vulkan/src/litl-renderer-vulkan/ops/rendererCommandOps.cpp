#include "litl-renderer-vulkan/renderer.hpp"

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
        }
        else
        {
            // Custom color attachment info
            TextureResource* colorTexture = vulkanContext->resources.getTexture(command.color.colorTexture);

            if (colorTexture != nullptr)
            {
                colorAttachment.imageView = colorTexture->vkImageView;
            }

            colorAttachment.loadOp = static_cast<VkAttachmentLoadOp>(command.color.loadOp);
            colorAttachment.storeOp = static_cast<VkAttachmentStoreOp>(command.color.storeOp);
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
            }

            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = depthTextureView;
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachment.resolveMode = VK_RESOLVE_MODE_NONE;
            depthAttachment.loadOp = static_cast<VkAttachmentLoadOp>((*command.depth).loadOp);
            depthAttachment.storeOp = static_cast<VkAttachmentStoreOp>((*command.depth).storeOp);
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
                    .width  = static_cast<uint32_t>(command.area.extents.x()),
                    .height = static_cast<uint32_t>(command.area.extents.y())
                }
            },
            .layerCount = command.layerCount,
            .viewMask = command.viewMask,
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
            .srcStageMask = static_cast<VkPipelineStageFlagBits2>(command.sourceStage),
            .srcAccessMask = static_cast<VkAccessFlags2>(command.sourceAccess),
            .dstStageMask = static_cast<VkPipelineStageFlagBits2>(command.destStage),
            .dstAccessMask = static_cast<VkAccessFlags2>(command.destAccess),
            .oldLayout = static_cast<VkImageLayout>(command.fromLayout),
            .newLayout = static_cast<VkImageLayout>(command.toLayout),
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
            static_cast<VkImageLayout>(command.destLayout),
            &clearColor, 
            1, 
            &range);
    }
}