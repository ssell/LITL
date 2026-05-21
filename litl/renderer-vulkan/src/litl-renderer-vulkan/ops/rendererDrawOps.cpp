#include <vector>

#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-core/constants.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // Begin Render
    // -------------------------------------------------------------------------------------

    bool isRenderReady(RendererContext& context, PerFrameSyncInfo const& frameSync) noexcept;
    bool acquireSwapChainIndex(RendererContext& context, PerFrameSyncInfo const& frameSync, uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) noexcept;

    /// <summary>
    /// Begins rendering if the last frame is complete.
    /// Returns false if the last frame is still in progress.
    /// </summary>
    /// <param name="context"></param>
    /// <returns></returns>
    bool beginRender(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);
        auto& frameSync = vulkanContext->getCurrFrameSyncInfo();

        if (!isRenderReady(*vulkanContext, frameSync))
        {
            // Fence not ready. Previous frame is still rendering.
            return false;
        }

        uint32_t swapChainImageIndex = 0;

        if (!acquireSwapChainIndex(*vulkanContext, frameSync, Constants::millisecond_to_nanoseconds * 32, vulkanContext->renderInfo.frameInFlightIndex, &swapChainImageIndex))
        {
            // Swapchain not ready.
            return false;
        }

        vulkanContext->swapChain.swapChainImageIndex = swapChainImageIndex;

        vkResetFences(vulkanContext->device.vkDevice, 1, &frameSync.renderFence);

        return true;
    }

    /// <summary>
    /// Checks if the render fence is open. If not, we are still rendering the last frame and need to wait.
    /// </summary>
    /// <param name="context"></param>
    /// <param name="frameSync"></param>
    /// <returns></returns>
    bool isRenderReady(RendererContext& context, PerFrameSyncInfo const& frameSync) noexcept
    {
        const VkResult fenceResult = vkGetFenceStatus(context.device.vkDevice, frameSync.renderFence);

        if (fenceResult != VK_SUCCESS)
        {
            // Fence is not signaled OR is in error state.
            if (fenceResult == VK_NOT_READY)
            {
                // Exit out of rendering so time can be spent on logic, etc.
                return false;
            }
            else
            {
                // todo log error
                return false;
            }
        }

        return true;
    }

    /// <summary>
    /// Acquires the index into our swapchain array for the next image to render to.
    /// </summary>
    /// <param name="context"></param>
    /// <param name="timeoutNs"></param>
    /// <param name="frameIndex"></param>
    /// <param name="imageIndex"></param>
    /// <returns></returns>
    bool acquireSwapChainIndex(RendererContext& context, PerFrameSyncInfo const& frameSync, uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) noexcept
    {
        const VkResult acquireResult = vkAcquireNextImageKHR(
            context.device.vkDevice,
            context.swapChain.vkSwapChain,
            timeoutNs,
            frameSync.presentCompleteSemaphore,
            VK_NULL_HANDLE,
            imageIndex);

        if ((acquireResult == VK_ERROR_OUT_OF_DATE_KHR) || (acquireResult == VK_SUBOPTIMAL_KHR) || context.window.wasResized)
        {
            recreateSwapchain(context);
            return false;
        }
        else
        {
            return (acquireResult == VK_SUCCESS);
        }
    }

    // -------------------------------------------------------------------------------------
    // Submit Draw Commands
    // -------------------------------------------------------------------------------------

    void submitCommands(litl::RendererContext* context, std::span<CommandBufferHandle const> commands) noexcept
    {
        if (commands.size() == 0)
        {
            return;
        }

        auto* vulkanContext = unwrap(context);

        // Build up command buffer info
        std::vector<VkCommandBufferSubmitInfo> vkCommandBuffers;
        vkCommandBuffers.reserve(commands.size());

        for (auto& commandBufferHandle : commands)
        {
            const auto* commandBufferResource = vulkanContext->resources.getCommandBuffer(commandBufferHandle);

            if ((commandBufferResource != nullptr) && (commandBufferResource->vkCommandBuffer != VK_NULL_HANDLE))
            {
                vkCommandBuffers.push_back(VkCommandBufferSubmitInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                    .commandBuffer = commandBufferResource->vkCommandBuffer
                });
            }
        }

        if (vkCommandBuffers.size() == 0)
        {
            return;
        }

        // Build up sync info
        auto& frameSync = vulkanContext->getCurrFrameSyncInfo();
        auto& imageSync = vulkanContext->getCurrImageSyncInfo();

        const auto waitDestinationStageMask = VkPipelineStageFlags2(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

        const VkSemaphoreSubmitInfo presentCompleteSemaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = frameSync.presentCompleteSemaphore,
            .stageMask = waitDestinationStageMask  // ?
        };

        const VkSemaphoreSubmitInfo renderCompleteSemaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = imageSync.renderCompleteSemaphore,
            .stageMask = waitDestinationStageMask  // ?
        };

        const VkSubmitInfo2 submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .flags = 0,
            .waitSemaphoreInfoCount = 1,
            .pWaitSemaphoreInfos = &presentCompleteSemaphoreInfo,
            .commandBufferInfoCount = static_cast<uint32_t>(vkCommandBuffers.size()),
            .pCommandBufferInfos = reinterpret_cast<VkCommandBufferSubmitInfo const*>(vkCommandBuffers.data()),
            .signalSemaphoreInfoCount = 1,
            .pSignalSemaphoreInfos = &renderCompleteSemaphoreInfo
        };

        // Submit
        const VkResult submitResult = vkQueueSubmit2(vulkanContext->device.vkGraphicsQueue, 1, &submitInfo, frameSync.renderFence);

        if (submitResult != VK_SUCCESS)
        {
            logWarning("Vulkan Renderer: vkQueueSubmit failed with result ", submitResult);
        }
    }

    // -------------------------------------------------------------------------------------
    // End Render
    // -------------------------------------------------------------------------------------

    void endRender(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        const auto presentInfo = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &vulkanContext->getCurrImageSyncInfo().renderCompleteSemaphore,  // Wait for the command buffer to finish executing
            .swapchainCount = 1,
            .pSwapchains = &vulkanContext->swapChain.vkSwapChain,
            .pImageIndices = &vulkanContext->swapChain.swapChainImageIndex,                     // Which image to draw onto
            .pResults = nullptr                                                                 // (Optional) If using multiple swapchains, the success of each present will be stored in this array as opposed to the singular result from the upcoming call.
        };

        const auto presentResult = vkQueuePresentKHR(vulkanContext->device.vkPresentQueue, &presentInfo);

        if (presentResult != VK_SUCCESS)
        {
            logWarning("Vulkan Renderer: vkQueuePresentKHR failed with result ", presentResult);
        }

        vulkanContext->renderInfo.frameCount++;
        vulkanContext->renderInfo.frameInFlightIndex = vulkanContext->renderInfo.frameCount % vulkanContext->renderInfo.framesInFlight;
    }
}