#ifndef LITL_VULKAN_RENDERER_CONTEXT_H__
#define LITL_VULKAN_RENDERER_CONTEXT_H__

#include <cstdint>
#include <vector>

#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/resourceManager.hpp"

namespace litl
{
    class Window;

    namespace vulkan
    {
        struct WindowInfo
        {
            /// <summary>
            /// The abstract window to which we are rendering.
            /// </summary>
            Window* window;

            /// <summary>
            /// Has the window been resized.
            /// </summary>
            bool wasResized;
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
            /// Memory allocator used by buffers and textures.
            /// </summary>
            VmaAllocator vmaAllocator = VK_NULL_HANDLE;

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
            uint32_t graphicsQueueIndex = 0u;

            /// <summary>
            /// The present command queue.
            /// </summary>
            VkQueue vkPresentQueue = VK_NULL_HANDLE;

            /// <summary>
            /// The present queue index.
            /// </summary>
            uint32_t presentQueueIndex = 0u;
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
            uint32_t swapChainImageIndex = 0u;

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
        struct PerFrameSyncInfo
        {
            CommandBufferHandle commandBuffer{};
            VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
            VkFence renderFence = VK_NULL_HANDLE;
        };

        /// <summary>
        /// For syncing an individual image.
        /// </summary>
        struct PerImageSyncInfo
        {
            VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
        };

        /// <summary>
        /// For syncinc.
        /// </summary>
        struct RenderInfo
        {
            /// <summary>
            /// Number of frames in flight.
            /// </summary>
            uint32_t framesInFlight = 2u;

            /// <summary>
            /// Total number of frames rendered
            /// </summary>
            uint32_t frameCount = 0u;

            /// <summary>
            /// The index of the current frame in flight.
            /// This is (frameCount % framesInFlight)
            /// </summary>
            uint32_t frameInFlightIndex = 0u;

            /// <summary>
            /// Indexed by FrameInfo::frameInFlight
            /// </summary>
            std::vector<PerFrameSyncInfo> frameSyncInfo;

            /// <summary>
            /// Indexed by the swapchain image index.
            /// </summary>
            std::vector<PerImageSyncInfo> imageSyncInfo;
        };

        struct RendererContext
        {
            WindowInfo window{};
            DeviceInfo device{};
            SwapChainInfo swapChain{};
            RenderInfo renderInfo{};
            ResourceManager resources;

            [[nodiscard]] PerFrameSyncInfo& getCurrFrameSyncInfo() noexcept
            {
                return renderInfo.frameSyncInfo[renderInfo.frameInFlightIndex];
            }

            [[nodiscard]] PerImageSyncInfo& getCurrImageSyncInfo() noexcept
            {
                return renderInfo.imageSyncInfo[swapChain.swapChainImageIndex];
            }
        };

        static RendererContext* unwrap(litl::RendererContext* opaqueContext) noexcept
        {
            return reinterpret_cast<RendererContext*>(opaqueContext);
        }

        static litl::RendererContext* wrap(RendererContext* concreteContext) noexcept
        {
            return reinterpret_cast<litl::RendererContext*>(concreteContext);
        }
    }
}

#endif