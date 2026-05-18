#include <set>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/math.hpp"
#include "litl-renderer/window.hpp"
#include "litl-renderer-vulkan/integration.hpp"
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
        auto* context = new RendererContext();

        context->window.window = pWindow;
        context->frame.framesInFlight = rendererDescriptor.framesInFlight;

        return new litl::Renderer(&litl::vulkan::VulkanRendererOps, context);
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
        VK_KHR_MAINTENANCE1_EXTENSION_NAME          // to allow negative viewport heights
    };


    // -------------------------------------------------------------------------------------
    // Creation
    // -------------------------------------------------------------------------------------

    bool createInstance(RendererContext* context) noexcept;
    bool createWindowSurface(RendererContext* context) noexcept;
    bool selectPhysicalDevice(RendererContext* context) noexcept;
    bool createLogicalDevice(RendererContext* context) noexcept;
    bool createSwapChain(RendererContext* context) noexcept;
    bool createCommandPool(RendererContext* context) noexcept;
    bool createSyncObjects(RendererContext* context) noexcept;

    bool build(RendererContext* context) noexcept
    {
        LITL_ASSERT_MSG(context->window.window != nullptr, "Vulkan Renderer requires a non-null Window. Headless rendering not yet supported.", false);

        return
            createInstance(context) &&
            createWindowSurface(context) &&
            selectPhysicalDevice(context) &&
            createLogicalDevice(context) &&
            createSwapChain(context) &&
            createCommandPool(context) &&
            createSyncObjects(context);
    }

    /// <summary>
    /// Creates the Vulkan instance that we will use.
    /// </summary>
    /// <returns></returns>
    bool createInstance(RendererContext* context) noexcept
    {
        // See: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "litl-engine";
        appInfo.pEngineName = "LITL";
        appInfo.apiVersion = VK_API_VERSION_1_4;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

#ifdef DEBUG
        if (!verifyValidationLayers())
        {
            return false;
        }

        createInfo.enabledLayerCount = static_cast<uint32_t>(RequiredValidationLayers.size());
        createInfo.ppEnabledLayerNames = RequiredValidationLayers.data();
        // by default these just print to standard out. can use callbacks if needed. see: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/02_Validation_layers.html#_message_callback
#endif

        VkResult result = vkCreateInstance(&createInfo, nullptr, &context->device.vkInstance);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Instance with result ", result);
            return false;
        }

        return true;
    }

    /// <summary>
    /// Ensures all required validation layers are present.
    /// </summary>
    /// <returns></returns>
    bool verifyValidationLayers(RendererContext* context) noexcept
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

        return true;
    }

    /// <summary>
    /// Creates the surface to which we will render.
    /// </summary>
    /// <returns></returns>
    bool createWindowSurface(RendererContext* context) noexcept
    {
        VkResult result = glfwCreateWindowSurface(
            context->device.vkInstance, 
            static_cast<GLFWwindow*>(context->window.window->getSurfaceWindow()), 
            nullptr, 
            &context->device.vkSurface);

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

        return requiredExtensions.empty();
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

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            checkPhysicalDeviceExtensionSupport(device) &&
            deviceFeatures.geometryShader;
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
    bool selectPhysicalDevice(RendererContext* context) noexcept
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(context->device.vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(context->device.vkInstance, &deviceCount, availableDevices.data());

        for (auto device : availableDevices)
        {
            auto queueFamilies = findQueueFamilies(device, context->device.vkSurface);

            if (isPhysicalDeviceSuitable(device) && queueFamilies.hasAll())
            {
                auto swapChainSupport = SwapChainSupport::querySwapChainSupport(device, context->device.vkSurface);

                if (swapChainSupport.isSwapChainSufficient())
                {
                    context->device.vkPhysicalDevice = device;
                    break;
                }
            }
        }

        return (context->device.vkPhysicalDevice != VK_NULL_HANDLE);
    }

    /// <summary>
    /// Creates the logical device and command queues.
    /// </summary>
    /// <returns></returns>
    bool createLogicalDevice(RendererContext* context) noexcept
    {
        auto queueFamilies = findQueueFamilies(context->device.vkPhysicalDevice, context->device.vkSurface);

        context->device.graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        context->device.presentQueueIndex = queueFamilies.getPresentIndex();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { context->device.graphicsQueueIndex, context->device.presentQueueIndex };
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
        auto vulkanDyanmicStateFeatures = VkPhysicalDeviceExtendedDynamicStateFeaturesEXT{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = nullptr,
            .extendedDynamicState = true
        };

        auto vulkan13Features = VkPhysicalDeviceVulkan13Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &vulkanDyanmicStateFeatures,
            .synchronization2 = true,
            .dynamicRendering = true
        };

        auto vulkan12Features = VkPhysicalDeviceVulkan12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = &vulkan13Features,
            .bufferDeviceAddress = VK_TRUE
        };

        auto vulkan11Features = VkPhysicalDeviceVulkan11Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = &vulkan12Features,
            .shaderDrawParameters = true
        };

        auto physicalDeviceFeatures = VkPhysicalDeviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &vulkan11Features,
            .features = VkPhysicalDeviceFeatures {
                .geometryShader = true
            }
        };

        const auto deviceCreateInfo = VkDeviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &physicalDeviceFeatures,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = RequiredDeviceExtensions.data()
        };

        const VkResult result = vkCreateDevice(context->device.vkPhysicalDevice, &deviceCreateInfo, nullptr, &context->device.vkDevice);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Device with result ", result);
            return false;
        }

        vkGetDeviceQueue(context->device.vkDevice, queueFamilies.getGraphicsIndex(), 0, &context->device.vkGraphicsQueue);
        vkGetDeviceQueue(context->device.vkDevice, queueFamilies.getPresentIndex(), 0, &context->device.vkPresentQueue);

        if (context->device.vkGraphicsQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieved Vulkan Graphics Queue");
            return false;
        }

        if (context->device.vkPresentQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieve Vulkan Present Queue");
            return false;
        }

        return true;
    }

    bool createSwapChain(RendererContext* context) noexcept
    {
        context->window.wasResized = false;

        int32_t frameBufferWidth = 0;
        int32_t frameBufferHeight = 0;

        glfwGetFramebufferSize(static_cast<GLFWwindow*>(context->window.window->getSurfaceWindow()), &frameBufferWidth, &frameBufferHeight);

        auto swapChainSupport = SwapChainSupport::querySwapChainSupport(context->device.vkPhysicalDevice, context->device.vkSurface);
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
        createSwapChainInfo.surface = context->device.vkSurface;
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
        createSwapChainInfo.oldSwapchain = VK_NULL_HANDLE;

        auto queueIndices = findQueueFamilies(context->device.vkPhysicalDevice, context->device.vkSurface);
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

        VkResult result = vkCreateSwapchainKHR(context->device.vkDevice, &createSwapChainInfo, nullptr, &context->swapChain.vkSwapChain);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Swap Chain with result ", result);
            return false;
        }

        context->swapChain.vkSwapChainImageFormat = surfaceFormat.format;
        context->swapChain.vkSwapChainExtent = imageExtent;

        vkGetSwapchainImagesKHR(context->device.vkDevice, context->swapChain.vkSwapChain, &imageCount, nullptr);
        context->swapChain.vkSwapChainImages.resize(imageCount);
        context->swapChain.vkSwapChainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(context->device.vkDevice, context->swapChain.vkSwapChain, &imageCount, context->swapChain.vkSwapChainImages.data());

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo createImageViewInfo{};
            createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createImageViewInfo.image = context->swapChain.vkSwapChainImages[i];
            createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createImageViewInfo.format = context->swapChain.vkSwapChainImageFormat;
            createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createImageViewInfo.subresourceRange.baseMipLevel = 0;
            createImageViewInfo.subresourceRange.levelCount = 1;
            createImageViewInfo.subresourceRange.baseArrayLayer = 0;
            createImageViewInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(context->device.vkDevice, &createImageViewInfo, nullptr, &context->swapChain.vkSwapChainImageViews[i]);

            if (result != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Swap Chain Image View with result ", result);
                return false;
            }
        }

        return true;
    }

    bool createCommandPool(RendererContext* context) noexcept
    {
        // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html

        const auto commandPoolInfo = VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            .queueFamilyIndex = context->device.graphicsQueueIndex
        };

        const VkResult result = vkCreateCommandPool(context->device.vkDevice, &commandPoolInfo, nullptr, &context->device.vkCommandPool);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Command Buffer Pool with result ", result);
            return false;
        }

        return true;
    }

    bool createSyncObjects(RendererContext* context) noexcept
    {
        const auto presentSemaphoreInfo = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        const auto renderSemaphoreInfo = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        const auto renderFenceInfo = VkFenceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        // Note that frameSync count is tied to FRAMES_IN_FLIGHT (which is currently 2)
        context->renderSync.frameSync.resize(context->frame.framesInFlight);

        for (size_t i = 0; i < context->frame.framesInFlight; ++i)
        {
            const auto presentCompleteSemaphoreResult = vkCreateSemaphore(context->device.vkDevice, &presentSemaphoreInfo, nullptr, &context->renderSync.frameSync[i].presentCompleteSemaphore);
            const auto renderFenceResult = vkCreateFence(context->device.vkDevice, &renderFenceInfo, nullptr, &context->renderSync.frameSync[i].renderFence);

            if (presentCompleteSemaphoreResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Present Complete Semaphore with result ", presentCompleteSemaphoreResult);
                return false;
            }

            if (renderFenceResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Render Fence with result ", renderFenceResult);
                return false;
            }
        }

        // Note that imageSync count is tied to swapchain image count (which is the device minimum + 1 (so 3 on this machine))
        context->renderSync.imageSync.resize(context->swapChain.vkSwapChainImages.size());

        for (size_t i = 0; i < context->swapChain.vkSwapChainImages.size(); ++i)
        {
            const auto renderCompleteSemaphoreResult = vkCreateSemaphore(context->device.vkDevice, &renderSemaphoreInfo, nullptr, &context->renderSync.imageSync[i].renderCompleteSemaphore);

            if (renderCompleteSemaphoreResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Render Complete Semaphore with result ", renderCompleteSemaphoreResult);
                return false;
            }
        }

        return true;
    }
    // -------------------------------------------------------------------------------------
    // Destruction
    // -------------------------------------------------------------------------------------

    void destroy(RendererContext* context) noexcept
    {
        // ... todo ...

        delete context;
    }
}