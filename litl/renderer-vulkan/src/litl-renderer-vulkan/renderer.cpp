#include <cstring>
#include <set>
#include <string>
#include <vector>

#include "litl-core/debug.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/common.hpp"
#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/queueFamily.hpp"
#include "litl-renderer-vulkan/swapChainSupport.hpp"

namespace litl
{
    /// <summary>
    /// Defined in integration.hpp and used by the RendererFactory in litl-engine
    /// </summary>
    /// <param name="pWindow"></param>
    /// <param name="rendererDescriptor"></param>
    /// <returns></returns>
    litl::Renderer* createVulkanRenderer(Window* pWindow, RendererConfiguration const& rendererDescriptor) noexcept
    {
        LITL_FATAL_ASSERT_MSG(pWindow != nullptr, "Attempting to create Vulkan Renderer with a null Window");

        vulkan::RendererContext* vulkanContext = new(std::nothrow) vulkan::RendererContext();

        vulkanContext->window.window = pWindow;
        vulkanContext->renderInfo.framesInFlight = rendererDescriptor.framesInFlight;

        return new litl::Renderer(&litl::vulkan::VulkanRendererOps, vulkan::wrap(vulkanContext));
    }

    void destroyVulkanRenderer(Renderer* renderer) noexcept
    {
        if (renderer != nullptr)
        {
            delete renderer;
        }
    }
}

namespace litl::vulkan
{
    // -------------------------------------------------------------------------------------
    // Required Features
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Validation layers are intermediate layers mainly used for debug purposes.
    /// Vulkan by default does very little error checking. Validation layers are
    /// ways to implement layer checking, profiling, etc.
    /// </summary>
    static const std::vector<const char*> RequiredValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    /// <summary>
    /// Extensions required by our renderer.
    /// </summary>
    static const std::vector<const char*> RequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_EXTENDED_DYNAMIC_STATE_3_EXTENSION_NAME      // dynamic rendering to allow null viewport in graphics pipeline, etc.
    };


    // -------------------------------------------------------------------------------------
    // Creation
    // -------------------------------------------------------------------------------------

    bool verifyValidationLayers() noexcept;
    bool initializeVolk() noexcept;
    bool createInstance(RendererContext& context) noexcept;
    bool createWindowSurface(RendererContext& context) noexcept;
    bool selectPhysicalDevice(RendererContext& context) noexcept;
    bool createLogicalDevice(RendererContext& context) noexcept;
    bool createMemoryAllocator(RendererContext& context) noexcept;
    bool createPipelineCache(RendererContext& context) noexcept;
    bool createResourceManager(RendererContext& context) noexcept;
    bool createSwapChain(RendererContext& context, VkSwapchainKHR oldSwapchain) noexcept;
    bool createCommandPool(RendererContext& context) noexcept;
    bool createFrameSyncObjects(RendererContext& context) noexcept;
    bool createImageSyncObjects(RendererContext& context) noexcept;

    bool build(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        LITL_ASSERT_MSG(vulkanContext->window.window != nullptr, "Vulkan Renderer requires a non-null Window. Headless rendering not yet supported.", false);

        return
            initializeVolk() &&
            verifyValidationLayers() &&
            createInstance(*vulkanContext) &&
            createWindowSurface(*vulkanContext) &&
            selectPhysicalDevice(*vulkanContext) &&
            createLogicalDevice(*vulkanContext) &&
            createMemoryAllocator(*vulkanContext) &&
            createPipelineCache(*vulkanContext) &&
            createResourceManager(*vulkanContext) &&
            createSwapChain(*vulkanContext, VK_NULL_HANDLE) &&
            createCommandPool(*vulkanContext) &&
            createFrameSyncObjects(*vulkanContext) &&
            createImageSyncObjects(*vulkanContext);
    }

    /// <summary>
    /// Initializes Volk.
    /// 
    /// Volk is a meta-loader for Vulkan that bypasses the default loader's trampoline functions.
    /// 
    /// When you call something like vkCmdDraw, you're not calling the driver directly. 
    /// You're calling into the loader, which dispatches through a trampoline that figures out 
    /// which ICD (Installable Client Driver) and which device the call belongs to, then forwards it. 
    /// For functions called millions of times per frame, that indirection adds up.
    /// 
    /// Volk solves this by loading Vulkan dynamically at runtime and resolving function pointers directly 
    /// from the driver, skipping the trampoline entirely.
    /// </summary>
    /// <returns></returns>
    bool initializeVolk() noexcept
    {
        // must be called before any vulkan call
        const VkResult result = volkInitialize();

        if (result != VK_SUCCESS)
        {
            logError("Failed to initialize Volk with result ", result);
            return false;
        }

        return true;
    }

    /// <summary>
    /// Ensures all required validation layers are present.
    /// </summary>
    /// <returns></returns>
    bool verifyValidationLayers() noexcept
    {
        if constexpr (LITL_DEBUG)
        {
            // See: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (uint32_t i = 0; i < RequiredValidationLayers.size(); ++i)
            {
                bool layerFound = false;

                for (uint32_t j = 0; j < layerCount; ++j)
                {
                    if (std::strcmp(RequiredValidationLayers[i], availableLayers[j].layerName) == 0)
                    {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound)
                {
                    return false;
                }
            }
        }

        return true;
    }

    /// <summary>
    /// Creates the Vulkan instance that we will use.
    /// </summary>
    /// <returns></returns>
    bool createInstance(RendererContext& context) noexcept
    {
        // See: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
        const VkApplicationInfo appInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "litl-engine",
            .pEngineName = "LITL",
            .apiVersion = LITL_VULKAN_VERSION,
        };

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = glfwExtensionCount,
            .ppEnabledExtensionNames = glfwExtensions,
        };

        if constexpr (LITL_DEBUG)
        {
            // by default these just print to standard out. can use callbacks if needed. see: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html#_message_callback
            createInfo.enabledLayerCount = static_cast<uint32_t>(RequiredValidationLayers.size());
            createInfo.ppEnabledLayerNames = RequiredValidationLayers.data();
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &context.device.vkInstance);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Instance with result ", result);
            return false;
        }

        // populate instance-level functions
        volkLoadInstance(context.device.vkInstance);

        return true;
    }

    /// <summary>
    /// Creates the surface to which we will render.
    /// </summary>
    /// <returns></returns>
    bool createWindowSurface(RendererContext& context) noexcept
    {
        VkResult result = glfwCreateWindowSurface(
            context.device.vkInstance, 
            static_cast<GLFWwindow*>(context.window.window->getSurfaceWindow()),
            nullptr, 
            &context.device.vkSurface);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Window Surface with result ", result);
            return false;
        }

        return true;
    }

    /// <summary>
    /// Checks if the physical device has all of our required extensions.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device) noexcept
    {
        uint32_t extensionCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(RequiredDeviceExtensions.begin(), RequiredDeviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        if (!requiredExtensions.empty())
        {
            logError("One or more required Vulkan extensions are not available on the physical device:");

            for (const auto& extension : requiredExtensions)
            {
                logError("Missing required extensions: ", extension);
            }

            return false;
        }

        return true;
    }

    /// <summary>
    /// Checks the capabilities of the physical device to determine if it is suitable for our needs.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    bool isPhysicalDeviceSuitable(VkPhysicalDevice device) noexcept
    {
        // Don't need this for these demos, but in reality see: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/03_Physical_devices_and_queue_families.html#_base_device_suitability_checks
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return
            ((deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) || (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)) &&
            checkPhysicalDeviceExtensionSupport(device) &&
            deviceFeatures.geometryShader &&
            deviceFeatures.tessellationShader;
            // TODO the full feature set will need to be specified by the engine/user/game/whatever
    }

    /// <summary>
    /// Finds all relevant queue families present on the given physical device.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="surface"></param>
    /// <returns></returns>
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) noexcept
    {
        QueueFamilyIndices indices{};

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        if (queueFamilyCount == 0)
        {
            return indices;
        }

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (auto i = 0; i < queueFamilies.size(); ++i)
        {
            if (!indices.hasGraphicsIndex() && (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            {
                indices.setGraphicsIndex(i);
            }

            if (!indices.hasPresentIndex())
            {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport)
                {
                    indices.setPresentIndex(i);
                }
            }

            if (indices.hasAll())
            {
                break;
            }
        }

        return indices;
    }

    /// <summary>
    /// Selects the best-fit physical device to integrate with.
    /// </summary>
    /// <returns></returns>
    bool selectPhysicalDevice(RendererContext& context) noexcept
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(context.device.vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(context.device.vkInstance, &deviceCount, availableDevices.data());

        for (auto device : availableDevices)
        {
            auto queueFamilies = findQueueFamilies(device, context.device.vkSurface);

            if (isPhysicalDeviceSuitable(device) && queueFamilies.hasAll())
            {
                auto swapChainSupport = SwapChainSupport::querySwapChainSupport(device, context.device.vkSurface);

                if (swapChainSupport.isSwapChainSufficient())
                {
                    context.device.vkPhysicalDevice = device;
                    break;
                }
            }
        }

        return (context.device.vkPhysicalDevice != VK_NULL_HANDLE);
    }

    /// <summary>
    /// Creates the logical device and command queues.
    /// </summary>
    /// <returns></returns>
    bool createLogicalDevice(RendererContext& context) noexcept
    {
        auto queueFamilies = findQueueFamilies(context.device.vkPhysicalDevice, context.device.vkSurface);

        context.device.graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        context.device.presentQueueIndex = queueFamilies.getPresentIndex();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { context.device.graphicsQueueIndex, context.device.presentQueueIndex };
        float queuePriorities[1] = { 1.0f };

        for (auto queueFamilyIndex : uniqueQueueFamilies)
        {
            queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamilyIndex,
                .queueCount = 1,
                .pQueuePriorities = queuePriorities,
                });
        }

        // query for Vulkan advanced feature set
        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT vulkanDynamicStateFeatures {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = nullptr,
            .extendedDynamicState = true
        };

        VkPhysicalDeviceVulkan13Features vulkan13Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &vulkanDynamicStateFeatures,
            .synchronization2 = true,
            .dynamicRendering = true,
        };

        VkPhysicalDeviceVulkan12Features vulkan12Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &vulkan13Features,
            .bufferDeviceAddress = VK_TRUE
        };

        VkPhysicalDeviceVulkan11Features vulkan11Features {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &vulkan12Features,
            .shaderDrawParameters = true
        };

        VkPhysicalDeviceFeatures2 physicalDeviceFeatures {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &vulkan11Features,
            .features = VkPhysicalDeviceFeatures {
                .geometryShader = true,
                .tessellationShader = true
            }
        };

        const VkDeviceCreateInfo deviceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &physicalDeviceFeatures,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = RequiredDeviceExtensions.data()
        };

        const VkResult result = vkCreateDevice(context.device.vkPhysicalDevice, &deviceCreateInfo, nullptr, &context.device.vkDevice);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Device with result ", result);
            return false;
        }

        // populate device-level functions
        volkLoadDevice(context.device.vkDevice);

        vkGetDeviceQueue(context.device.vkDevice, queueFamilies.getGraphicsIndex(), 0, &context.device.vkGraphicsQueue);
        vkGetDeviceQueue(context.device.vkDevice, queueFamilies.getPresentIndex(), 0, &context.device.vkPresentQueue);

        if (context.device.vkGraphicsQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieved Vulkan Graphics Queue");
            return false;
        }

        if (context.device.vkPresentQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieve Vulkan Present Queue");
            return false;
        }

        return true;
    }

    bool createMemoryAllocator(RendererContext& context) noexcept
    {
        const VmaVulkanFunctions vulkanFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr
        };

        const VmaAllocatorCreateInfo createVmaInfo{
            .flags = 
                VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | 
                VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,              // notify we are using VkBufferUsageFlags2
            .physicalDevice = context.device.vkPhysicalDevice,
            .device = context.device.vkDevice,
            .pVulkanFunctions = &vulkanFunctions,                       // wire together with Volk
            .instance = context.device.vkInstance,
            .vulkanApiVersion = LITL_VULKAN_VERSION
        };

        const VkResult result = vmaCreateAllocator(&createVmaInfo, &context.device.vmaAllocator);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Memory Allocator with result ", result);
            return false;
        }

        return true;
    }

    bool createSwapChain(RendererContext& context, VkSwapchainKHR oldSwapchain) noexcept
    {
        context.window.wasResized = false;

        int32_t frameBufferWidth = 0;
        int32_t frameBufferHeight = 0;

        glfwGetFramebufferSize(static_cast<GLFWwindow*>(context.window.window->getSurfaceWindow()), &frameBufferWidth, &frameBufferHeight);

        auto swapChainSupport = SwapChainSupport::querySwapChainSupport(context.device.vkPhysicalDevice, context.device.vkSurface);
        auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
        auto presentMode = swapChainSupport.chooseSwapPresentMode();
        auto imageExtent = swapChainSupport.chooseSwapExtent(frameBufferWidth, frameBufferHeight);
        auto imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0)        // == 0 means no limit
        {
            imageCount = (min)(imageCount, swapChainSupport.capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createSwapChainInfo{};
        createSwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createSwapChainInfo.surface = context.device.vkSurface;
        createSwapChainInfo.minImageCount = imageCount;
        createSwapChainInfo.imageFormat = surfaceFormat.format;
        createSwapChainInfo.imageColorSpace = surfaceFormat.colorSpace;
        createSwapChainInfo.imageExtent = imageExtent;
        createSwapChainInfo.imageArrayLayers = 1;
        createSwapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;    // for postprocessing see VK_IMAGE_USAGE_TRANSFER_DST_BIT (https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/01_Presentation/01_Swap_chain.html#_creating_the_swap_chain)
        createSwapChainInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createSwapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createSwapChainInfo.presentMode = presentMode;
        createSwapChainInfo.clipped = VK_TRUE;
        createSwapChainInfo.oldSwapchain = oldSwapchain;

        auto queueIndices = findQueueFamilies(context.device.vkPhysicalDevice, context.device.vkSurface);
        uint32_t queueFamilyIndices[] = { queueIndices.getGraphicsIndex(), queueIndices.getPresentIndex() };

        if (queueIndices.getGraphicsIndex() != queueIndices.getPresentIndex())
        {
            createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createSwapChainInfo.queueFamilyIndexCount = 2;
            createSwapChainInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createSwapChainInfo.queueFamilyIndexCount = 0;
            createSwapChainInfo.pQueueFamilyIndices = nullptr;
        }

        VkResult result = vkCreateSwapchainKHR(context.device.vkDevice, &createSwapChainInfo, nullptr, &context.swapChain.vkSwapChain);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Swap Chain with result ", result);
            return false;
        }

        context.swapChain.vkSwapChainImageFormat = surfaceFormat.format;
        context.swapChain.vkSwapChainExtent = imageExtent;

        vkGetSwapchainImagesKHR(context.device.vkDevice, context.swapChain.vkSwapChain, &imageCount, nullptr);
        context.swapChain.vkSwapChainImages.resize(imageCount);
        context.swapChain.vkSwapChainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(context.device.vkDevice, context.swapChain.vkSwapChain, &imageCount, context.swapChain.vkSwapChainImages.data());

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            const VkImageViewCreateInfo createImageViewInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = context.swapChain.vkSwapChainImages[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = context.swapChain.vkSwapChainImageFormat,
                .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
                .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .subresourceRange.baseMipLevel = 0,
                .subresourceRange.levelCount = 1,
                .subresourceRange.baseArrayLayer = 0,
                .subresourceRange.layerCount = 1
            };

            result = vkCreateImageView(context.device.vkDevice, &createImageViewInfo, nullptr, &context.swapChain.vkSwapChainImageViews[i]);

            if (result != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Swap Chain Image View with result ", result);
                return false;
            }
        }

        return true;
    }

    bool createCommandPool(RendererContext& context) noexcept
    {
        // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html

        const VkCommandPoolCreateInfo commandPoolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            .queueFamilyIndex = context.device.graphicsQueueIndex
        };

        const VkResult result = vkCreateCommandPool(context.device.vkDevice, &commandPoolInfo, nullptr, &context.device.vkCommandPool);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Command Buffer Pool with result ", result);
            return false;
        }

        return true;
    }

    bool createFrameSyncObjects(RendererContext& context) noexcept
    {
        const VkSemaphoreCreateInfo presentSemaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        const VkFenceCreateInfo renderFenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        // Note that frameSync count is tied to FRAMES_IN_FLIGHT (which is currently 2)
        context.renderInfo.frameSyncInfo.resize(context.renderInfo.framesInFlight);

        for (size_t i = 0; i < context.renderInfo.framesInFlight; ++i)
        {
            auto& frameSyncInfo = context.renderInfo.frameSyncInfo[i];

            // Command Buffer
            frameSyncInfo.commandBuffer = context.resources.createCommandBuffer({});

            // Present Semaphore
            const auto presentCompleteSemaphoreResult = vkCreateSemaphore(context.device.vkDevice, &presentSemaphoreInfo, nullptr, &frameSyncInfo.presentCompleteSemaphore);

            if (presentCompleteSemaphoreResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Present Complete Semaphore with result ", presentCompleteSemaphoreResult);
                return false;
            }

            // Render Fence
            const auto renderFenceResult = vkCreateFence(context.device.vkDevice, &renderFenceInfo, nullptr, &frameSyncInfo.renderFence);

            if (renderFenceResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Render Fence with result ", renderFenceResult);
                return false;
            }

            // Staging Ring Buffer
            frameSyncInfo.stagingRingBuffer.allocateFixedBuffer();
        }

        return true;
    }

    bool createImageSyncObjects(RendererContext& context) noexcept
    {
        const VkSemaphoreCreateInfo renderSemaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        // Note that imageSync count is tied to swapchain image count (which is the device minimum + 1 (so 3 on this machine))
        context.renderInfo.imageSyncInfo.clear();
        context.renderInfo.imageSyncInfo.resize(context.swapChain.vkSwapChainImages.size());

        for (size_t i = 0; i < context.swapChain.vkSwapChainImages.size(); ++i)
        {
            const auto renderCompleteSemaphoreResult = vkCreateSemaphore(context.device.vkDevice, &renderSemaphoreInfo, nullptr, &context.renderInfo.imageSyncInfo[i].renderCompleteSemaphore);

            if (renderCompleteSemaphoreResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Render Complete Semaphore with result ", renderCompleteSemaphoreResult);
                return false;
            }
        }

        return true;
    }

    bool createPipelineCache(RendererContext& context) noexcept
    {
        // TODO save/load cache to disk mechanism
        const VkPipelineCacheCreateInfo cacheInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .initialDataSize = 0,
            .pInitialData = nullptr
        };

        const VkResult result = vkCreatePipelineCache(context.device.vkDevice, &cacheInfo, nullptr, &context.device.vkPipelineCache);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Pipeline Cache with result ", result);
            return false;
        }

        return true;
    }

    bool createResourceManager(RendererContext& context) noexcept
    {
        context.resources.build(context);
        return true;
    }

    // -------------------------------------------------------------------------------------
    // Destruction
    // -------------------------------------------------------------------------------------

    void cleanupResources(RendererContext& context) noexcept;
    void cleanupPipelineCache(RendererContext& context) noexcept;
    void cleanupFrameSync(RendererContext& context) noexcept;
    void cleanupImageSync(RendererContext& context) noexcept;
    void cleanupSwapChainImages(RendererContext& context) noexcept;
    void cleanupSwapChain(RendererContext& context, VkSwapchainKHR swapchain) noexcept;
    void cleanupDevice(RendererContext& context) noexcept;
    void recreateSwapchain(RendererContext& context) noexcept;

    void destroy(litl::RendererContext* context) noexcept
    {
        auto* vulkanContext = unwrap(context);

        vkDeviceWaitIdle(vulkanContext->device.vkDevice);

        cleanupResources(*vulkanContext);
        cleanupPipelineCache(*vulkanContext);
        cleanupFrameSync(*vulkanContext);
        cleanupImageSync(*vulkanContext);
        cleanupSwapChainImages(*vulkanContext);
        cleanupSwapChain(*vulkanContext, vulkanContext->swapChain.vkSwapChain);
        cleanupDevice(*vulkanContext);

        delete vulkanContext;
    }

    void cleanupResources(RendererContext& context) noexcept
    {
        context.resources.destroy();
    }

    void cleanupPipelineCache(RendererContext& context) noexcept
    {
        if (context.device.vkPipelineCache != VK_NULL_HANDLE)
        {
            vkDestroyPipelineCache(context.device.vkDevice, context.device.vkPipelineCache, nullptr);
            context.device.vkPipelineCache = VK_NULL_HANDLE;
        }
    }

    void cleanupFrameSync(RendererContext& context) noexcept
    {
        for (auto& frameInfo : context.renderInfo.frameSyncInfo)
        {
            context.resources.destroyCommandBuffer(frameInfo.commandBuffer);
            frameInfo.commandBuffer = {};

            if (frameInfo.presentCompleteSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(context.device.vkDevice, frameInfo.presentCompleteSemaphore, nullptr);
            }

            if (frameInfo.renderFence != VK_NULL_HANDLE)
            {
                vkDestroyFence(context.device.vkDevice, frameInfo.renderFence, nullptr);
            }
        }

        context.renderInfo.frameSyncInfo.clear();
    }

    void cleanupImageSync(RendererContext& context) noexcept
    {
        for (auto& imageInfo : context.renderInfo.imageSyncInfo)
        {
            if (imageInfo.renderCompleteSemaphore != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(context.device.vkDevice, imageInfo.renderCompleteSemaphore, nullptr);
            }
        }

        context.renderInfo.imageSyncInfo.clear();
    }

    void cleanupSwapChainImages(RendererContext& context) noexcept
    {
        if (!context.swapChain.vkSwapChainImageViews.empty())
        {
            // Note: only have to destroy the views since we explicitly made them, and not the underlying images.
            // Those will be destroyed alongside the swap chain itself since it made them.
            for (auto imageView : context.swapChain.vkSwapChainImageViews)
            {
                vkDestroyImageView(context.device.vkDevice, imageView, nullptr);
            }
        }
    }

    void cleanupSwapChain(RendererContext& context, VkSwapchainKHR swapchain) noexcept
    {
        if (swapchain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(context.device.vkDevice, swapchain, nullptr);
        }
    }

    void cleanupDevice(RendererContext& context) noexcept
    {
        if (context.device.vkCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(context.device.vkDevice, context.device.vkCommandPool, nullptr);
        }

        if (context.device.vkSurface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(context.device.vkInstance, context.device.vkSurface, nullptr);
        }

        if (context.device.vmaAllocator != VK_NULL_HANDLE)
        {
            vmaDestroyAllocator(context.device.vmaAllocator);
        }

        if (context.device.vkDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(context.device.vkDevice, nullptr);
        }

        if (context.device.vkInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(context.device.vkInstance, nullptr);
        }
    }

    void waitForValidFramebufferSize(RendererContext& context) noexcept
    {
        while (context.window.window->getWidth() == 0u || context.window.window->getHeight() == 0u)
        {
            context.window.window->waitForEvents();
        }
    }

    void recreateSwapchain(RendererContext& context) noexcept
    {
        // Wait for a valid recreation state
        waitForValidFramebufferSize(context);           // wait for frame buffer to have non-zero dimensions (minimized, etc.)
        vkDeviceWaitIdle(context.device.vkDevice);      // wait for all resources to be free

        VkSwapchainKHR oldSwapchain = context.swapChain.vkSwapChain;

        cleanupSwapChainImages(context);
        createSwapChain(context, oldSwapchain);         // pass in the old swapchain to make creating the new one
        cleanupSwapChain(context, oldSwapchain);        // destroy the old one

        // Swapchain image count _can_ change. So must recreate the image sync objects.
        cleanupImageSync(context);
        createImageSyncObjects(context);
    }
}