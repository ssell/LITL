#ifndef LITL_VULKAN_RENDERER_CONTEXT_H__
#define LITL_VULKAN_RENDERER_CONTEXT_H__

#include <cstdint>
#include <vector>

#include "litl-renderer/renderer.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/resourceManager.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingBuffer.hpp"
#include "litl-renderer-vulkan/resources/utility/stagingTexture.hpp"
#include "litl-renderer-vulkan/resources/utility/descriptorSetAllocator.hpp"

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
            /// Command buffer pool for long-lived buffers.
            /// </summary>
            VkCommandPool vkCommandPool = VK_NULL_HANDLE;

            /// <summary>
            /// Command buffer pool for short-lived, single-submission buffers.
            /// </summary>
            VkCommandPool vkCommandPoolTransient = VK_NULL_HANDLE;

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

            /// <summary>
            /// The transfer command queue.
            /// </summary>
            VkQueue vkTransferQueue = VK_NULL_HANDLE;

            /// <summary>
            /// The transfer queue index.
            /// </summary>
            uint32_t transferQueueIndex = 0u;

            /// <summary>
            /// Pipeline cache objects allow the result of pipeline construction to be reused between pipelines and between runs of an application.
            /// </summary>
            VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
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
            /// <summary>
            /// Shared command buffer for the frame.
            /// </summary>
            CommandBufferHandle commandBuffer{};

            /// <summary>
            /// Fence to signal the end of rendering.
            /// </summary>
            VkFence renderFence = VK_NULL_HANDLE;

            /// <summary>
            /// Semaphore to signal the end of presentation.
            /// </summary>
            VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;

            /// <summary>
            /// Holds both fixed and temporary staging buffers for GPU uploads.
            /// Cleared out at the start of each frame.
            /// 
            /// Note: this is stored in an unique_ptr since PerFrameSyncInfo is itself stored in a vector.
            /// </summary>
            std::unique_ptr<StagingBuffer> stagingBufferArena;

            /// <summary>
            /// Holds temporary staging textures for GPU uploads.
            /// Cleared out at the start of each frame.
            /// 
            /// Note: this is stored in an unique_ptr since PerFrameSyncInfo is itself stored in a vector.
            /// </summary>
            std::unique_ptr<StagingTexture> stagingTextureArena;
            
            /// <summary>
            /// Holds the per-frame descriptor set allocator (collection of pools).
            /// Note: this is stored in an unique_ptr since PerFrameSyncInfo is itself stored in a vector.
            /// </summary>
            std::unique_ptr<DescriptorSetAllocator> descriptorSetAllocator;
        };

        /// <summary>
        /// For syncing an individual image.
        /// </summary>
        struct PerImageSyncInfo
        {
            VkSemaphore renderCompleteSemaphore = VK_NULL_HANDLE;
        };

        /// <summary>
        /// Information about the current draw command(s).
        /// Populated whenever cmdBeginRender is called.
        /// </summary>
        struct DrawInfo
        {
            VkExtent2D targetTextureSize = { 0, 0 };
            VkFormat depthFormat = VkFormat::VK_FORMAT_UNDEFINED;
            VkFormat stencilFormat = VkFormat::VK_FORMAT_UNDEFINED;
        };

        /// <summary>
        /// For syncinc.
        /// </summary>
        struct RenderInfo
        {
            /// <summary>
            /// Frame-specific data.
            /// </summary>
            FrameData frame{};

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
            RendererConfiguration config{};
            WindowInfo window{};
            DeviceInfo device{};
            SwapChainInfo swapChain{};
            RenderInfo renderInfo{};
            DrawInfo drawInfo{};
            ResourceManager resources;

            [[nodiscard]] PerFrameSyncInfo& getCurrFrameSyncInfo() noexcept
            {
                return renderInfo.frameSyncInfo[renderInfo.frame.frameInFlightIndex];
            }

            [[nodiscard]] PerFrameSyncInfo& getPrevFrameSyncInfo() noexcept
            {
                const uint32_t prevFrameInFlightIndex = (renderInfo.frame.frameCount + (renderInfo.frame.framesInFlight - 1)) % renderInfo.frame.framesInFlight;
                return renderInfo.frameSyncInfo[prevFrameInFlightIndex];
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