#ifndef LITL_VULKAN_RENDERER_CONTEXT_H__
#define LITL_VULKAN_RENDERER_CONTEXT_H__

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace litl
{
    class Window;

    struct WindowInfo
    {
        /// <summary>
        /// The abstract window to which we are rendering.
        /// </summary>
        Window* pWindow;

        /// <summary>
        /// Has the window been resized.
        /// </summary>
        bool wasResized;
    };

    struct FrameInfo
    {
        /// <summary>
        /// Number of frames in flight.
        /// </summary>
        uint32_t framesInFlight = 2;

        /// <summary>
        /// Total number of frames rendered
        /// </summary>
        uint32_t frameCount = 0;

        /// <summary>
        /// The index of the current frame in flight.
        /// This is (frameCount % framesInFlight)
        /// </summary>
        uint32_t frameInFlight = 0;
    };

    struct DeviceInfo
    {
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
        /// Command buffer pooo.
        /// </summary>
        VkCommandPool vkCommandPool = VK_NULL_HANDLE;

        /// <summary>
        /// The graphics command queue.
        /// </summary>
        VkQueue vkGraphicsQueue = VK_NULL_HANDLE;

        /// <summary>
        /// The graphics queue index.
        /// </summary>
        uint32_t graphicsQueueIndex = 0;

        /// <summary>
        /// The present command queue.
        /// </summary>
        VkQueue vkPresentQueue = VK_NULL_HANDLE;

        /// <summary>
        /// The present queue index.
        /// </summary>
        uint32_t presentQueueIndex = 0;
    };

    struct SwapChainInfo
    {
        /// <summary>
        /// Our swap chain.
        /// </summary>
        VkSwapchainKHR vkSwapChain = VK_NULL_HANDLE;

        /// <summary>
        /// The format of the swap chain images.
        /// </summary>
        VkFormat vkSwapChainImageFormat;

        /// <summary>
        /// The size of the swap chain images.
        /// </summary>
        VkExtent2D vkSwapChainExtent;

        /// <summary>
        /// The currently secured swapchain image index.
        /// </summary>
        uint32_t swapChainImageIndex = 0;

        /// <summary>
        /// The images in the swap chain.
        /// </summary>
        std::vector<VkImage> vkSwapChainImages;

        /// <summary>
        /// The views into the swap chain images.
        /// </summary>
        std::vector<VkImageView> vkSwapChainImageViews;
    };

    /// <summary>
    /// For syncing an entire frame.
    /// </summary>
    struct FrameSyncInfo
    {
        VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
        VkFence renderFence = VK_NULL_HANDLE;
    };

    /// <summary>
    /// For syncing an individual image.
    /// </summary>
    struct ImageSyncInfo
    {
        VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
    };

    /// <summary>
    /// For syncinc.
    /// </summary>
    struct RenderSyncInfo
    {
        /// <summary>
        /// Indexed by FrameInfo::frameInFlight
        /// </summary>
        std::vector<FrameSyncInfo> frameSync;

        /// <summary>
        /// Indexed by the swapchain image index.
        /// </summary>
        std::vector<ImageSyncInfo> vkRenderCompleteSemaphores;
    };

    struct RendererContext
    {
        WindowInfo window{};
        FrameInfo frame{};
        DeviceInfo device{};
        SwapChainInfo swapChain{};
        RenderSyncInfo renderSync{};
    };
}

#endif