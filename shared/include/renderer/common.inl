#ifndef LITL_COMMON_IMPL_H__
#define LITL_COMMON_IMPL_H__

#include <vulkan/vulkan.h>
#include <vector>

#include "renderer/common.hpp"

namespace LITL::Renderer
{
    static const uint32_t FRAMES_IN_FLIGHT = 2;

    struct RenderContext
    {
        /// <summary>
        /// Frame count.
        /// </summary>
        uint32_t frame = 0;

        /// <summary>
        /// Was the framebuffer/window resized?
        /// </summary>
        bool wasResized = false;

        /// <summary>
        /// Our window. Typically a GLFWwindow.
        /// </summary>
        void* window = nullptr;

        /// <summary>
        /// Connection to the Vulkan library.
        /// </summary>
        VkInstance vkInstance = VK_NULL_HANDLE;

        /// <summary>
        /// A specific physical device (GPU).
        /// </summary>
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        /// <summary>
        /// Logical interface to the physical device.
        /// </summary>
        VkDevice device = VK_NULL_HANDLE;

        /// <summary>
        /// The window surface on which to render.
        /// </summary>
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        /// <summary>
        /// Our graphics command queue.
        /// </summary>
        VkQueue graphicsQueue = VK_NULL_HANDLE;

        /// <summary>
        /// Our graphics queue index.
        /// </summary>
        uint32_t graphicsQueueIndex = 0;

        /// <summary>
        /// Our present command queue.
        /// </summary>
        VkQueue presentQueue = VK_NULL_HANDLE;

        /// <summary>
        /// Our present queue index.
        /// </summary>
        uint32_t presentQueueIndex = 0;

        /// <summary>
        /// Our swap chain.
        /// </summary>
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;

        /// <summary>
        /// The images in the swap chain.
        /// </summary>
        std::vector<VkImage> swapChainImages;

        /// <summary>
        /// The views into the swap chain images.
        /// </summary>
        std::vector<VkImageView> swapChainImageViews;

        /// <summary>
        /// The format of the swap chain images.
        /// </summary>
        VkFormat swapChainImageFormat;

        /// <summary>
        /// The size of the swap chain images.
        /// </summary>
        VkExtent2D swapChainExtent;

        /// <summary>
        /// The fixed function pipeline layout.
        /// </summary>
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        /// <summary>
        /// The fixed function pipeline.
        /// </summary>
        VkPipeline pipeline = VK_NULL_HANDLE;

        /// <summary>
        /// Manage memory that is used to store command buffers.
        /// </summary>
        VkCommandPool commandPool = VK_NULL_HANDLE;

        /// <summary>
        /// Semaphore (GPU) that is signaled when we are done presenting the swapchain image to the screen.
        /// One semaphore per swapchain image.
        /// </summary>
        std::vector<VkSemaphore> presentCompleteSemaphores;

        /// <summary>
        /// Semaphore (GPU) that is signaled when the swapchain image is done being drawn.
        /// One semaphore per swapchain image.
        /// </summary>
        std::vector<VkSemaphore> renderCompleteSemaphores;

        /// <summary>
        /// Fence (CPU) that is signaled when an entire frame is complete.
        /// One fence per swapchain image.
        /// </summary>
        std::vector<VkFence> renderFences;

        /// <summary>
        /// Returns the current frame index, which is NOT the frame count.
        /// This is the index into the command buffers, swapchain images, semaphores, etc. that should be used this frame.
        /// </summary>
        /// <returns></returns>
        constexpr uint32_t getFrameIndex() const
        {
            return frame % FRAMES_IN_FLIGHT;
        }
    };

    struct ShaderContext
    {
        VkDevice device = VK_NULL_HANDLE;
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VkPipelineShaderStageCreateInfo shaderStageInfo;
    };

    struct CommandBufferContext
    {
        VkDevice device = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        /// <summary>
        /// One or more command buffers in this context. The number of command buffers is set to FRAMES_IN_FLIGHT.
        /// </summary>
        std::vector<VkCommandBuffer> commandBuffers;

        VkCommandBuffer getCurrentCommandBuffer(RenderContext const* pRenderContext) const
        {
            return commandBuffers[pRenderContext->getFrameIndex()];
        }
    };
}

#endif