#ifndef LITL_VULKAN_RENDER_CONTEXT_H__
#define LITL_VULKAN_RENDER_CONTEXT_H__

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vector>

#include "litl-core/window.hpp"

namespace LITL::Vulkan::Renderer
{
    /// <summary>
    /// The context of a Vulkan Renderer.
    /// 
    /// This is exposed in this header as it makes passing around certain core Vulkan objects easier.
    /// For example, the Vulkan ResourceAllocator requires access to many of the same objects as the Renderer itself.
    /// </summary>
    struct RendererContext
    {
        Core::Window* pWindow;

        /// <summary>
        /// Frame count.
        /// </summary>
        uint32_t frame = 0;

        /// <summary>
        /// frame % framesInFlight. Used for selecting resources that alternate between frames.
        /// </summary>
        uint32_t frameIndex = 0;

        /// <summary>
        /// The currently secured swapchain image index.
        /// </summary>
        uint32_t swapChainImageIndex = 0;

        /// <summary>
        /// Number of frames in flight.
        /// </summary>
        uint32_t framesInFlight = 2;

        /// <summary>
        /// Was the framebuffer/window resized?
        /// </summary>
        bool wasResized = false;

        /// <summary>
        /// Our window surface. Typically a GLFWwindow.
        /// </summary>
        void* pSurfaceWindow = nullptr;

        /// <summary>
        /// Connection to the Vulkan library.
        /// </summary>
        VkInstance vkInstance = VK_NULL_HANDLE;

        /// <summary>
        /// A specific physical device (GPU).
        /// </summary>
        VkPhysicalDevice vkPhysicalDevice = VK_NULL_HANDLE;

        /// <summary>
        /// Logical interface to the physical device.
        /// </summary>
        VkDevice vkDevice = VK_NULL_HANDLE;

        /// <summary>
        /// The window surface on which to render.
        /// </summary>
        VkSurfaceKHR vkSurface = VK_NULL_HANDLE;

        /// <summary>
        /// Our graphics command queue.
        /// </summary>
        VkQueue vkGraphicsQueue = VK_NULL_HANDLE;

        /// <summary>
        /// Our graphics queue index.
        /// </summary>
        uint32_t graphicsQueueIndex = 0;

        /// <summary>
        /// Our present command queue.
        /// </summary>
        VkQueue vkPresentQueue = VK_NULL_HANDLE;

        /// <summary>
        /// Our present queue index.
        /// </summary>
        uint32_t presentQueueIndex = 0;

        /// <summary>
        /// Our swap chain.
        /// </summary>
        VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;

        /// <summary>
        /// The images in the swap chain.
        /// </summary>
        std::vector<VkImage> vkSwapChainImages;

        /// <summary>
        /// The views into the swap chain images.
        /// </summary>
        std::vector<VkImageView> vkSwapChainImageViews;

        /// <summary>
        /// The format of the swap chain images.
        /// </summary>
        VkFormat vkSwapChainImageFormat;

        /// <summary>
        /// The size of the swap chain images.
        /// </summary>
        VkExtent2D vkSwapChainExtent;

        /// <summary>
        /// Manage memory that is used to store command buffers.
        /// </summary>
        VkCommandPool vkCommandPool = VK_NULL_HANDLE;

        /// <summary>
        /// Semaphore (GPU) that is signaled when we are done presenting the swapchain image to the screen.
        /// One semaphore per swapchain image.
        /// </summary>
        std::vector<VkSemaphore> vkPresentCompleteSemaphores;

        /// <summary>
        /// Semaphore (GPU) that is signaled when the swapchain image is done being drawn.
        /// One semaphore per swapchain image.
        /// </summary>
        std::vector<VkSemaphore> vkRenderCompleteSemaphores;

        /// <summary>
        /// Fence (CPU) that is signaled when an entire frame is complete.
        /// One fence per swapchain image.
        /// </summary>
        std::vector<VkFence> vkRenderFences;
    };

    struct RendererHandle
    {
        RendererContext context;
    };
}

#endif