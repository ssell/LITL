#include <set>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer/pipeline/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/queueFamily.hpp"
#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/renderContext.hpp"
#include "litl-renderer-vulkan/swapchainSupport.hpp"

namespace LITL::Vulkan::Renderer
{
    struct Renderer::Impl
    {
        Core::Window* pWindow;
        RenderContext* pContext;

        ~Impl()
        {
            if (pContext != nullptr)
            {
                delete pContext;
            }
        }
    };

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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // -------------------------------------------------------------------------------------
    // Renderer Creation
    // -------------------------------------------------------------------------------------

    LITL::Renderer::Renderer* createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor)
    {
        return dynamic_cast<LITL::Renderer::Renderer*>(new Renderer(pWindow, rendererDescriptor));
    }

    Renderer::Renderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor)
    {
        m_impl->pWindow = pWindow;
        m_impl->pContext = new RenderContext{
            .framesInFlight = rendererDescriptor.framesInFlight
        };
    }

    Renderer::~Renderer()
    {

    }

    bool Renderer::initialize() const noexcept
    {
        if (m_impl->pWindow == nullptr)
        {
            logError("Vulkan Window provided to Vulkan Renderer is null.");
            return false;
        }

        return createInstance() &&
            createWindowSurface() &&
            selectPhysicalDevice() &&
            createLogicalDevice() &&
            createSwapChain() &&
            createCommandPool() &&
            createSyncObjects();
    }

    // -------------------------------------------------------------------------------------
    // Creation
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates the Vulkan instance that we will use.
    /// </summary>
    /// <param name="applicationName"></param>
    /// <returns></returns>
    bool Renderer::createInstance() const
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

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_impl->pContext->vkInstance);

        if (result != VK_SUCCESS)
        {
            logCritical("Failed to create Vulkan Instance with result ", result);
            return false;
        }

        return true;
    }

    /// <summary>
    /// Ensures all required validation layers are present.
    /// </summary>
    /// <returns></returns>
    bool Renderer::verifyValidationLayers() const
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
    bool Renderer::createWindowSurface() const
    {
        VkResult result = glfwCreateWindowSurface(m_impl->pContext->vkInstance, static_cast<GLFWwindow*>(m_impl->pContext->window), nullptr, &m_impl->pContext->surface);

        if (result != VK_SUCCESS)
        {
            logCritical("Failed to create Vulkan Window Surface with result ", result);
            return false;
        }

        return true;
    }

    /// <summary>
    /// Checks if the physical device has all of our required extensions.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    static bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice device)
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
    static bool isPhysicalDeviceSuitable(VkPhysicalDevice device)
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
    static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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
    bool Renderer::selectPhysicalDevice() const
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_impl->pContext->vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_impl->pContext->vkInstance, &deviceCount, availableDevices.data());

        for (auto device : availableDevices)
        {
            auto queueFamilies = findQueueFamilies(device, m_impl->pContext->surface);

            if (isPhysicalDeviceSuitable(device) && queueFamilies.hasAll())
            {
                auto swapChainSupport = SwapChainSupport::querySwapChainSupport(device, m_impl->pContext->surface);

                if (swapChainSupport.isSwapChainSufficient())
                {
                    m_impl->pContext->physicalDevice = device;
                    break;
                }
            }
        }

        return (m_impl->pContext->physicalDevice != VK_NULL_HANDLE);
    }

    /// <summary>
    /// Creates the logical device and command queues.
    /// </summary>
    /// <returns></returns>
    bool Renderer::createLogicalDevice() const
    {
        auto queueFamilies = findQueueFamilies(m_impl->pContext->physicalDevice, m_impl->pContext->surface);

        m_impl->pContext->graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        m_impl->pContext->presentQueueIndex = queueFamilies.getPresentIndex();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_impl->pContext->graphicsQueueIndex, m_impl->pContext->presentQueueIndex };
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
            .pNext = &vulkan13Features
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

        const VkResult result = vkCreateDevice(m_impl->pContext->physicalDevice, &deviceCreateInfo, nullptr, &m_impl->pContext->device);

        if (result != VK_SUCCESS)
        {
            // todo log out
            return false;
        }

        vkGetDeviceQueue(m_impl->pContext->device, queueFamilies.getGraphicsIndex(), 0, &m_impl->pContext->graphicsQueue);
        vkGetDeviceQueue(m_impl->pContext->device, queueFamilies.getPresentIndex(), 0, &m_impl->pContext->presentQueue);

        if ((m_impl->pContext->graphicsQueue == VK_NULL_HANDLE) || (m_impl->pContext->presentQueue == VK_NULL_HANDLE))
        {
            // todo log out
            return false;
        }

        return true;
    }

    bool Renderer::createSwapChain() const
    {
        m_impl->pContext->wasResized = false;

        int32_t frameBufferWidth = 0;
        int32_t frameBufferHeight = 0;

        glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_impl->pContext->window), &frameBufferWidth, &frameBufferHeight);

        auto swapChainSupport = SwapChainSupport::querySwapChainSupport(m_impl->pContext->physicalDevice, m_impl->pContext->surface);
        auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
        auto presentMode = swapChainSupport.chooseSwapPresentMode();
        auto imageExtent = swapChainSupport.chooseSwapExtent(frameBufferWidth, frameBufferHeight);
        auto imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0)        // == 0 means no limit
        {
            imageCount = Math::minimum(imageCount, swapChainSupport.capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createSwapChainInfo{};
        createSwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createSwapChainInfo.surface = m_impl->pContext->surface;
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

        auto queueIndices = findQueueFamilies(m_impl->pContext->physicalDevice, m_impl->pContext->surface);
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

        VkResult result = vkCreateSwapchainKHR(m_impl->pContext->device, &createSwapChainInfo, nullptr, &m_impl->pContext->swapChain);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        m_impl->pContext->swapChainImageFormat = surfaceFormat.format;
        m_impl->pContext->swapChainExtent = imageExtent;

        vkGetSwapchainImagesKHR(m_impl->pContext->device, m_impl->pContext->swapChain, &imageCount, nullptr);
        m_impl->pContext->swapChainImages.resize(imageCount);
        m_impl->pContext->swapChainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(m_impl->pContext->device, m_impl->pContext->swapChain, &imageCount, m_impl->pContext->swapChainImages.data());

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo createImageViewInfo{};
            createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createImageViewInfo.image = m_impl->pContext->swapChainImages[i];
            createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createImageViewInfo.format = m_impl->pContext->swapChainImageFormat;
            createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createImageViewInfo.subresourceRange.baseMipLevel = 0;
            createImageViewInfo.subresourceRange.levelCount = 1;
            createImageViewInfo.subresourceRange.baseArrayLayer = 0;
            createImageViewInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(m_impl->pContext->device, &createImageViewInfo, nullptr, &m_impl->pContext->swapChainImageViews[i]);

            if (result != VK_SUCCESS)
            {
                // todo log error
                return false;
            }
        }

        return true;
    }

    bool Renderer::createCommandPool() const
    {
        // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html

        const auto commandPoolInfo = VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            .queueFamilyIndex = m_impl->pContext->graphicsQueueIndex
        };

        const VkResult result = vkCreateCommandPool(m_impl->pContext->device, &commandPoolInfo, nullptr, &m_impl->pContext->commandPool);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        return true;
    }

    bool Renderer::createSyncObjects() const
    {
        if (m_impl->pContext->presentCompleteSemaphores.size() > 0)
        {
            return false;
        }

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

        // Note that renderCompleteSemaphores are tied to swapchain image count (which is the device minimum + 1 (so 3 on this machine))
        // And that presentCompleteSemaphores and renderFences are tied to FRAMES_IN_FLIGHT (which is currently 2)

        m_impl->pContext->renderCompleteSemaphores.resize(m_impl->pContext->swapChainImages.size(), VK_NULL_HANDLE);
        m_impl->pContext->presentCompleteSemaphores.resize(m_impl->pContext->framesInFlight, VK_NULL_HANDLE);
        m_impl->pContext->renderFences.resize(m_impl->pContext->framesInFlight, VK_NULL_HANDLE);

        for (size_t i = 0; i < m_impl->pContext->swapChainImages.size(); ++i)
        {
            const auto renderCompleteSemaphoreResult = vkCreateSemaphore(m_impl->pContext->device, &renderSemaphoreInfo, nullptr, &m_impl->pContext->renderCompleteSemaphores[i]);

            if (renderCompleteSemaphoreResult != VK_SUCCESS)
            {
                // todo log error
                return false;
            }
        }

        for (size_t i = 0; i < m_impl->pContext->framesInFlight; ++i)
        {
            const auto presentCompleteSemaphoreResult = vkCreateSemaphore(m_impl->pContext->device, &presentSemaphoreInfo, nullptr, &m_impl->pContext->presentCompleteSemaphores[i]);
            const auto renderFenceResult = vkCreateFence(m_impl->pContext->device, &renderFenceInfo, nullptr, &m_impl->pContext->renderFences[i]);

            if ((presentCompleteSemaphoreResult != VK_SUCCESS) || (renderFenceResult != VK_SUCCESS))
            {
                // todo log error
                return false;
            }
        }

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Object Creation
    // -------------------------------------------------------------------------------------

    // ... shader
    // ... command buffer
    // ... mesh

    // -------------------------------------------------------------------------------------
    // Pipeline Creation
    // -------------------------------------------------------------------------------------
}