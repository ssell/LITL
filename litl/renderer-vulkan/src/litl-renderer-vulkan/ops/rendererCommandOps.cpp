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

    RendererResult cmdBufferWrite(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle bufferHandle, void* source, uint64_t size, uint64_t destOffset, PipelineStageFlag bufferTargetStage) noexcept
    {
        if (source == nullptr)
        {
            return RendererResult::NullSource;
        }

        if (size == 0)
        {
            return RendererResult::ZeroSizedSource;
        }

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* bufferResource = vulkanContext->resources.getBuffer(bufferHandle);

        if (bufferResource == nullptr)
        {
            return RendererResult::InvalidBufferHandle;
        }

        // Based on the "Advanced data upload" example from: https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html
        const VkBufferUsageFlags2CreateInfo bufferCreate2Info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
            .usage = toVkBufferUsageFlag(bufferResource->descriptor.type)
        };

        if (bufferResource->allocationInfo.pMappedData != nullptr)
        {
            // Mapped buffer, can write directly to it.
            VkResult result = vmaCopyMemoryToAllocation(vulkanContext->device.vmaAllocator, source, bufferResource->allocation, static_cast<VkDeviceSize>(destOffset), size);     // copies AND flushes

            if (result != VK_SUCCESS)
            {
                return RendererResult::MemoryCopyFailed;
            }

            // Barrier to make sure the copy has finished
            const VkBufferMemoryBarrier memoryBarrier{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .pNext = &bufferCreate2Info,
                .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                //.dstAccessMask = supplied in pNext
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = bufferResource->vkBuffer,
                .offset = static_cast<VkDeviceSize>(destOffset),
                .size = VK_WHOLE_SIZE
            };

            vkCmdPipelineBarrier(
                commandBuffer->vkCommandBuffer,             // command buffer
                VK_PIPELINE_STAGE_HOST_BIT,                 // source stage mask
                toVkPipelineStageFlag(bufferTargetStage),   // dest stage mask
                0,                                          // dependency flags
                0,                                          // memory barrier count
                nullptr,                                    // memory barriers
                1,                                          // buffer memory barrier count
                &memoryBarrier,                             // buffer memory barriers
                0,                                          // image memory barrier count
                nullptr);                                   // image memory barriers
        }
        else
        {
            // Has a staging buffer that needs to be written to first, and then copied over.
            auto* stagingBufferResource = vulkanContext->resources.getBuffer(bufferResource->stagingBuffer);

            if ((stagingBufferResource != nullptr) && (stagingBufferResource->allocationInfo.pMappedData != nullptr))
            {
                // Write to staging buffer
                VkResult result = vmaCopyMemoryToAllocation(vulkanContext->device.vmaAllocator, source, stagingBufferResource->allocation, 0, size);

                if (result != VK_SUCCESS)
                {
                    return RendererResult::MemoryCopyFailed;
                }

                // Barrier to make sure the write to the staging buffer is complete
                const VkBufferMemoryBarrier memoryBarrier1{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = stagingBufferResource->vkBuffer,
                    .offset = static_cast<VkDeviceSize>(destOffset),
                    .size = VK_WHOLE_SIZE
                };

                vkCmdPipelineBarrier(
                    commandBuffer->vkCommandBuffer,         // command buffer
                    VK_PIPELINE_STAGE_HOST_BIT,             // source stage mask
                    VK_PIPELINE_STAGE_TRANSFER_BIT,         // dest stage mask
                    0,                                      // dependency flags
                    0,                                      // memory barrier count
                    nullptr,                                // memory barriers
                    1,                                      // buffer memory barrier count
                    &memoryBarrier1,                        // buffer memory barriers
                    0,                                      // image memory barrier count
                    nullptr);                               // image memory barriers

                // Copy to the target buffer
                VkBufferCopy bufferCopy{
                    .srcOffset = 0,
                    .dstOffset = static_cast<VkDeviceSize>(destOffset),
                    .size = static_cast<VkDeviceSize>(size)
                };

                vkCmdCopyBuffer(commandBuffer->vkCommandBuffer, stagingBufferResource->vkBuffer, bufferResource->vkBuffer, 1, &bufferCopy);

                // Barrier to make sure the copy has finished
                const VkBufferMemoryBarrier memoryBarrier2{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                    .pNext = &bufferCreate2Info,
                    .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    //.dstAccessMask = supplied in pNext
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .buffer = bufferResource->vkBuffer,
                    .offset = static_cast<VkDeviceSize>(destOffset),
                    .size = VK_WHOLE_SIZE
                };

                vkCmdPipelineBarrier(
                    commandBuffer->vkCommandBuffer,             // command buffer
                    VK_PIPELINE_STAGE_TRANSFER_BIT,             // source stage mask
                    toVkPipelineStageFlag(bufferTargetStage),   // dest stage mask
                    0,                                          // dependency flags
                    0,                                          // memory barrier count
                    nullptr,                                    // memory barriers
                    1,                                          // buffer memory barrier count
                    &memoryBarrier2,                            // buffer memory barriers
                    0,                                          // image memory barrier count
                    nullptr);                                   // image memory barriers
            }
            else
            {
                // Can not be directly written to and does not have an associated staging buffer. Invalid buffer for writing.
                return RendererResult::InvalidBufferForWriting;
            }
        }

        return RendererResult::Success;
    }

    RendererResult cmdBufferWriteIndirect(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle stagingBufferHandle, BufferHandle destBufferHandle, void* source, uint64_t size, uint64_t destOffset, PipelineStageFlag bufferTargetStage) noexcept
    {
        if (source == nullptr)
        {
            return RendererResult::NullSource;
        }

        if (size == 0)
        {
            return RendererResult::ZeroSizedSource;
        }

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* destBuffer = vulkanContext->resources.getBuffer(destBufferHandle);

        if (destBuffer == nullptr)
        {
            return RendererResult::InvalidBufferForWriting;
        }

        auto* stagingBuffer = vulkanContext->resources.getBuffer(stagingBufferHandle);
        bool temporaryStagingBuffer = false;

        if (stagingBuffer == nullptr)
        {
            // Allocate a temporary staging buffer.
            // TODO in the future we can pool/reuse staging buffers.

            BufferDescriptor stagingBufferDescriptor{
                .type = BufferTypeFlagBits::TransferSource,
                .memoryUsage = BufferMemoryUsage::Staging,
                .bytes = size
            };

            stagingBufferHandle = vulkanContext->resources.createBuffer(stagingBufferDescriptor);       // ????? destBuffer->vkBuffer getting invalidated somehow when returning from this ?????
            stagingBuffer = vulkanContext->resources.getBuffer(stagingBufferHandle);

            if (stagingBuffer == nullptr)
            {
                return RendererResult::StagingBufferFailure;
            }

            temporaryStagingBuffer = true;
        }

        // ---- Write to the staging buffer
        VkResult result = vmaCopyMemoryToAllocation(vulkanContext->device.vmaAllocator, source, stagingBuffer->allocation, 0, size);

        if (result != VK_SUCCESS)
        {
            return RendererResult::MemoryCopyFailed;
        }

        // ---- Barrier to make sure the write to the staging buffer is complete
        const VkBufferMemoryBarrier memoryBarrier1{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = stagingBuffer->vkBuffer,
            .offset = static_cast<VkDeviceSize>(destOffset),
            .size = VK_WHOLE_SIZE
        };

        vkCmdPipelineBarrier(
            commandBuffer->vkCommandBuffer,         // command buffer
            VK_PIPELINE_STAGE_HOST_BIT,             // source stage mask
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // dest stage mask
            0,                                      // dependency flags
            0,                                      // memory barrier count
            nullptr,                                // memory barriers
            1,                                      // buffer memory barrier count
            &memoryBarrier1,                        // buffer memory barriers
            0,                                      // image memory barrier count
            nullptr);                               // image memory barriers

        // ---- Copy to the target buffer
        VkBufferCopy bufferCopy{
            .srcOffset = 0,
            .dstOffset = static_cast<VkDeviceSize>(destOffset),
            .size = static_cast<VkDeviceSize>(size)
        };

        vkCmdCopyBuffer(commandBuffer->vkCommandBuffer, stagingBuffer->vkBuffer, destBuffer->vkBuffer, 1, &bufferCopy);

        // ---- Barrier to make sure the copy has finished
        const VkBufferUsageFlags2CreateInfo bufferCreate2Info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
            .usage = toVkBufferUsageFlag(destBuffer->descriptor.type)
        };

        const VkBufferMemoryBarrier memoryBarrier2{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = &bufferCreate2Info,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            //.dstAccessMask = supplied in pNext
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = destBuffer->vkBuffer,
            .offset = static_cast<VkDeviceSize>(destOffset),
            .size = VK_WHOLE_SIZE
        };

        vkCmdPipelineBarrier(
            commandBuffer->vkCommandBuffer,             // command buffer
            VK_PIPELINE_STAGE_TRANSFER_BIT,             // source stage mask
            toVkPipelineStageFlag(bufferTargetStage),   // dest stage mask
            0,                                          // dependency flags
            0,                                          // memory barrier count
            nullptr,                                    // memory barriers
            1,                                          // buffer memory barrier count
            &memoryBarrier2,                            // buffer memory barriers
            0,                                          // image memory barrier count
            nullptr);                                   // image memory barriers

        // Cleanup
        if (temporaryStagingBuffer)
        {
            // need a delayed destroy ...
            //vulkanContext->resources.destroyBuffer(stagingBufferHandle);
        }

        return RendererResult::Success;
    }

    RendererResult cmdBufferCopyInto(litl::RendererContext* context, CommandBufferHandle commandBufferHandle, BufferHandle sourceBufferHandle, BufferHandle destBufferHandle, uint64_t size, uint64_t sourceOffset, uint64_t destOffset, PipelineStageFlag bufferTargetStage) noexcept
    {
        if (size == 0)
        {
            return RendererResult::ZeroSizedSource;
        }

        auto* vulkanContext = unwrap(context);
        auto* commandBuffer = unwrapCommandBuffer(context, commandBufferHandle);

        if (!isValid(commandBuffer))
        {
            return RendererResult::InvalidCommandBufferHandle;
        }

        auto* sourceBuffer = vulkanContext->resources.getBuffer(sourceBufferHandle);

        if (sourceBuffer == nullptr)
        {
            return RendererResult::InvalidBufferForReading;
        }

        auto* destBuffer = vulkanContext->resources.getBuffer(destBufferHandle);

        if (destBuffer == nullptr)
        {
            return RendererResult::InvalidBufferForWriting;
        }

        // Copy from source into destination
        VkBufferCopy bufferCopy{
            .srcOffset = static_cast<VkDeviceSize>(sourceOffset),
            .dstOffset = static_cast<VkDeviceSize>(destOffset),
            .size = static_cast<VkDeviceSize>(size)
        };

        vkCmdCopyBuffer(commandBuffer->vkCommandBuffer, sourceBuffer->vkBuffer, destBuffer->vkBuffer, 1, &bufferCopy);

        // Barrier to make sure the copy has finished
        const VkBufferUsageFlags2CreateInfo bufferCreate2Info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_USAGE_FLAGS_2_CREATE_INFO,
            .usage = toVkBufferUsageFlag(destBuffer->descriptor.type)
        };

        const VkBufferMemoryBarrier memoryBarrier{
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = &bufferCreate2Info,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            //.dstAccessMask = supplied in pNext
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = destBuffer->vkBuffer,
            .offset = static_cast<VkDeviceSize>(destOffset),
            .size = static_cast<VkDeviceSize>(size)
        };

        vkCmdPipelineBarrier(
            commandBuffer->vkCommandBuffer,             // command buffer
            VK_PIPELINE_STAGE_TRANSFER_BIT,             // source stage mask
            toVkPipelineStageFlag(bufferTargetStage),   // dest stage mask
            0,                                          // dependency flags
            0,                                          // memory barrier count
            nullptr,                                    // memory barriers
            1,                                          // buffer memory barrier count
            &memoryBarrier,                             // buffer memory barriers
            0,                                          // image memory barrier count
            nullptr);                                   // image memory barriers

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