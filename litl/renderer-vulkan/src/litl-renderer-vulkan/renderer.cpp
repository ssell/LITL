#include <set>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-core/math/math.hpp"
#include "litl-renderer/pipeline/graphicsPipeline.hpp"
#include "litl-renderer-vulkan/queueFamily.hpp"
#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/swapchainSupport.hpp"
#include "litl-renderer-vulkan/commands/commandBuffer.hpp"
#include "litl-renderer-vulkan/pipeline/pipelineLayout.hpp"

namespace LITL::Vulkan::Renderer
{
    struct Renderer::Impl
    {
        Core::Window* pWindow;

        /// <summary>
        /// Frame count.
        /// </summary>
        uint32_t frame = 0;

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

    std::unique_ptr<LITL::Renderer::Renderer> createVulkanRenderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor)
    {
        return std::make_unique<Renderer>(pWindow, rendererDescriptor);
    }

    Renderer::Renderer(Core::Window* pWindow, LITL::Renderer::RendererDescriptor const& rendererDescriptor)
        : m_pImpl(std::make_unique<Impl>())
    {
        m_pImpl->pWindow = pWindow;
        m_pImpl->pSurfaceWindow = pWindow->getSurfaceWindow();
        m_pImpl->framesInFlight = rendererDescriptor.framesInFlight;
    }

    Renderer::~Renderer()
    {
        cleanup();
    }

    bool Renderer::initialize() noexcept
    {
        if (m_pImpl->pWindow == nullptr)
        {
            logError("Vulkan Window provided to Vulkan Renderer is null.");
            return false;
        }

        return 
            createInstance() &&
            createWindowSurface() &&
            selectPhysicalDevice() &&
            createLogicalDevice() &&
            createSwapChain() &&
            createCommandPool() &&
            createSyncObjects();
    }

    uint32_t Renderer::getFrame() const noexcept
    {
        return m_pImpl->frame;
    }

    uint32_t Renderer::getFrameIndex() const noexcept
    {
        return (m_pImpl->frame % m_pImpl->framesInFlight);
    }

    // -------------------------------------------------------------------------------------
    // Cleanup
    // -------------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------------

    void Renderer::cleanup()
    {
        for (auto i = 0; i < m_pImpl->vkPresentCompleteSemaphores.size(); ++i)
        {
            if (m_pImpl->vkPresentCompleteSemaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(m_pImpl->vkDevice, m_pImpl->vkPresentCompleteSemaphores[i], nullptr);
            }

            m_pImpl->vkPresentCompleteSemaphores.clear();
        }

        for (auto i = 0; i < m_pImpl->vkRenderCompleteSemaphores.size(); ++i)
        {
            if (m_pImpl->vkRenderCompleteSemaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(m_pImpl->vkDevice, m_pImpl->vkRenderCompleteSemaphores[i], nullptr);
            }

            m_pImpl->vkRenderCompleteSemaphores.clear();
        }

        for (auto i = 0; i < m_pImpl->vkRenderFences.size(); ++i)
        {
            if (m_pImpl->vkRenderFences[i] != VK_NULL_HANDLE)
            {
                vkDestroyFence(m_pImpl->vkDevice, m_pImpl->vkRenderFences[i], nullptr);
            }

            m_pImpl->vkRenderFences.clear();
        }

        if (m_pImpl->vkCommandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_pImpl->vkDevice, m_pImpl->vkCommandPool, nullptr);
        }

        cleanupSwapchain();

        if (m_pImpl->vkSurface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_pImpl->vkInstance, m_pImpl->vkSurface, nullptr);
        }

        if (m_pImpl->vkDevice != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_pImpl->vkDevice, nullptr);
        }

        if (m_pImpl->vkInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_pImpl->vkInstance, nullptr);
        }
    }

    void Renderer::cleanupSwapchain()
    {
        if (!m_pImpl->vkSwapChainImageViews.empty())
        {
            // Note: only have to destroy the views since we explicitly made them, and not the underlying images.
            // Those will be destroyed alongside the swap chain itself since it made them.
            for (auto imageView : m_pImpl->vkSwapChainImageViews)
            {
                vkDestroyImageView(m_pImpl->vkDevice, imageView, nullptr);
            }
        }

        if (m_pImpl->vkSwapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_pImpl->vkDevice, m_pImpl->vkSwapChain, nullptr);
        }
    }

    void Renderer::recreateSwapchain()
    {
        // Wait for our resources to be unused.
        vkDeviceWaitIdle(m_pImpl->vkDevice);

        cleanupSwapchain();
        createSwapChain();
    }

    // -------------------------------------------------------------------------------------
    // Creation
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates the Vulkan instance that we will use.
    /// </summary>
    /// <param name="applicationName"></param>
    /// <returns></returns>
    bool Renderer::createInstance()
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

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_pImpl->vkInstance);

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
    bool Renderer::verifyValidationLayers()
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
    bool Renderer::createWindowSurface()
    {
        VkResult result = glfwCreateWindowSurface(m_pImpl->vkInstance, static_cast<GLFWwindow*>(m_pImpl->pSurfaceWindow), nullptr, &m_pImpl->vkSurface);

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
    bool Renderer::selectPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_pImpl->vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_pImpl->vkInstance, &deviceCount, availableDevices.data());

        for (auto device : availableDevices)
        {
            auto queueFamilies = findQueueFamilies(device, m_pImpl->vkSurface);

            if (isPhysicalDeviceSuitable(device) && queueFamilies.hasAll())
            {
                auto swapChainSupport = SwapChainSupport::querySwapChainSupport(device, m_pImpl->vkSurface);

                if (swapChainSupport.isSwapChainSufficient())
                {
                    m_pImpl->vkPhysicalDevice = device;
                    break;
                }
            }
        }

        return (m_pImpl->vkPhysicalDevice != VK_NULL_HANDLE);
    }

    /// <summary>
    /// Creates the logical device and command queues.
    /// </summary>
    /// <returns></returns>
    bool Renderer::createLogicalDevice()
    {
        auto queueFamilies = findQueueFamilies(m_pImpl->vkPhysicalDevice, m_pImpl->vkSurface);

        m_pImpl->graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        m_pImpl->presentQueueIndex = queueFamilies.getPresentIndex();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_pImpl->graphicsQueueIndex, m_pImpl->presentQueueIndex };
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

        const VkResult result = vkCreateDevice(m_pImpl->vkPhysicalDevice, &deviceCreateInfo, nullptr, &m_pImpl->vkDevice);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Device with result ", result);
            return false;
        }

        vkGetDeviceQueue(m_pImpl->vkDevice, queueFamilies.getGraphicsIndex(), 0, &m_pImpl->vkGraphicsQueue);
        vkGetDeviceQueue(m_pImpl->vkDevice, queueFamilies.getPresentIndex(), 0, &m_pImpl->vkPresentQueue);

        if (m_pImpl->vkGraphicsQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieved Vulkan Graphics Queue");
            return false;
        }

        if (m_pImpl->vkPresentQueue == VK_NULL_HANDLE)
        {
            logError("Failed to retrieve Vulkan Present Queue");
            return false;
        }

        return true;
    }

    bool Renderer::createSwapChain()
    {
        m_pImpl->wasResized = false;

        int32_t frameBufferWidth = 0;
        int32_t frameBufferHeight = 0;

        glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_pImpl->pSurfaceWindow), &frameBufferWidth, &frameBufferHeight);

        auto swapChainSupport = SwapChainSupport::querySwapChainSupport(m_pImpl->vkPhysicalDevice, m_pImpl->vkSurface);
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
        createSwapChainInfo.surface = m_pImpl->vkSurface;
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

        auto queueIndices = findQueueFamilies(m_pImpl->vkPhysicalDevice, m_pImpl->vkSurface);
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

        VkResult result = vkCreateSwapchainKHR(m_pImpl->vkDevice, &createSwapChainInfo, nullptr, &m_pImpl->vkSwapChain);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Swap Chain with result ", result);
            return false;
        }

        m_pImpl->vkSwapChainImageFormat = surfaceFormat.format;
        m_pImpl->vkSwapChainExtent = imageExtent;

        vkGetSwapchainImagesKHR(m_pImpl->vkDevice, m_pImpl->vkSwapChain, &imageCount, nullptr);
        m_pImpl->vkSwapChainImages.resize(imageCount);
        m_pImpl->vkSwapChainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(m_pImpl->vkDevice, m_pImpl->vkSwapChain, &imageCount, m_pImpl->vkSwapChainImages.data());

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo createImageViewInfo{};
            createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createImageViewInfo.image = m_pImpl->vkSwapChainImages[i];
            createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createImageViewInfo.format = m_pImpl->vkSwapChainImageFormat;
            createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createImageViewInfo.subresourceRange.baseMipLevel = 0;
            createImageViewInfo.subresourceRange.levelCount = 1;
            createImageViewInfo.subresourceRange.baseArrayLayer = 0;
            createImageViewInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(m_pImpl->vkDevice, &createImageViewInfo, nullptr, &m_pImpl->vkSwapChainImageViews[i]);

            if (result != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Swap Chain Image View with result ", result);
                return false;
            }
        }

        return true;
    }

    bool Renderer::createCommandPool()
    {
        // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/03_Drawing/01_Command_buffers.html

        const auto commandPoolInfo = VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            .queueFamilyIndex = m_pImpl->graphicsQueueIndex
        };

        const VkResult result = vkCreateCommandPool(m_pImpl->vkDevice, &commandPoolInfo, nullptr, &m_pImpl->vkCommandPool);

        if (result != VK_SUCCESS)
        {
            logError("Failed to create Vulkan Command Buffer Pool with result ", result);
            return false;
        }

        return true;
    }

    bool Renderer::createSyncObjects()
    {
        if (m_pImpl->vkPresentCompleteSemaphores.size() > 0)
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

        m_pImpl->vkRenderCompleteSemaphores.resize(m_pImpl->vkSwapChainImages.size(), VK_NULL_HANDLE);
        m_pImpl->vkPresentCompleteSemaphores.resize(m_pImpl->framesInFlight, VK_NULL_HANDLE);
        m_pImpl->vkRenderFences.resize(m_pImpl->framesInFlight, VK_NULL_HANDLE);

        for (size_t i = 0; i < m_pImpl->vkSwapChainImages.size(); ++i)
        {
            const auto renderCompleteSemaphoreResult = vkCreateSemaphore(m_pImpl->vkDevice, &renderSemaphoreInfo, nullptr, &m_pImpl->vkRenderCompleteSemaphores[i]);

            if (renderCompleteSemaphoreResult != VK_SUCCESS)
            {
                logError("Failed to create Vulkan Render Complete Semaphore with result ", renderCompleteSemaphoreResult);
                return false;
            }
        }

        for (size_t i = 0; i < m_pImpl->framesInFlight; ++i)
        {
            const auto presentCompleteSemaphoreResult = vkCreateSemaphore(m_pImpl->vkDevice, &presentSemaphoreInfo, nullptr, &m_pImpl->vkPresentCompleteSemaphores[i]);
            const auto renderFenceResult = vkCreateFence(m_pImpl->vkDevice, &renderFenceInfo, nullptr, &m_pImpl->vkRenderFences[i]);

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

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Object Creation
    // -------------------------------------------------------------------------------------

    std::unique_ptr<LITL::Renderer::PipelineLayout> Renderer::createPipelineLayout(LITL::Renderer::PipelineLayoutDescriptor const& descriptor) const noexcept
    {
        auto pipelineLayout = std::make_unique<PipelineLayout>(m_pImpl->vkDevice, descriptor);

        if (!pipelineLayout->build())
        {
            logError("Failed to construct new Vulkan Pipeline Layout object");
            return nullptr;
        }

        return pipelineLayout;
    }

    std::unique_ptr<LITL::Renderer::CommandBuffer> Renderer::createCommandBuffer() const noexcept
    {
        auto commandBuffer = std::make_unique<CommandBuffer>(m_pImpl->vkDevice, m_pImpl->vkCommandPool, m_pImpl->framesInFlight);

        if (!commandBuffer->build())
        {
            logError("Failed to construct new Vulkan Command Buffer object");
            return nullptr;
        }

        return commandBuffer;
    }

    // -------------------------------------------------------------------------------------
    // Pipeline Creation
    // -------------------------------------------------------------------------------------

    // -------------------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------------------

    void Renderer::render(LITL::Renderer::CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers)
    {
        // ---------------------------------------------------------------------------------
        // Wait & Prepare 

        if (!isRenderReady())
        {
            return;
        }

        const auto frameIndex = getFrameIndex();
        uint32_t swapChainImageIndex = 0;

        if (!acquireSwapChainIndex(1000000, frameIndex, &swapChainImageIndex))                              // 1000000 ns = 1 ms
        {
            return;
        }

        // ---------------------------------------------------------------------------------
        // Record Commands

        CommandBuffer* vulkanCommandBuffers = dynamic_cast<CommandBuffer*>(pCommandBuffers);

        vkResetFences(m_pImpl->vkDevice, 1, &m_pImpl->vkRenderFences[frameIndex]);
        recordCommandBuffers(vulkanCommandBuffers, numCommandBuffers, swapChainImageIndex);

        // ---------------------------------------------------------------------------------
        // Submit Commands

        const auto waitDestinationStageMask = VkPipelineStageFlags(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Only 1 expected command buffer atm. If we move to actually have multiple, reconsider redesigning CommandBuffer for AoS vs SoA.
        const auto vkCommandBuffer = (&vulkanCommandBuffers)[0]->getCurrentCommandBuffer(m_pImpl->frame);

        const auto submitInfo = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_pImpl->vkPresentCompleteSemaphores[frameIndex],              // Wait for current swapchain image to be done presenting
            .pWaitDstStageMask = &waitDestinationStageMask,                                     // Wait for writing colors to the image until's available
            .commandBufferCount = 1,
            .pCommandBuffers = &vkCommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_pImpl->vkRenderCompleteSemaphores[swapChainImageIndex]     // The semaphore to signal once the command buffer(s) have finished execution.
        };

        const auto submitResult = vkQueueSubmit(m_pImpl->vkGraphicsQueue, 1, &submitInfo, m_pImpl->vkRenderFences[frameIndex]);      // Submit the command buffer

        if (submitResult != VK_SUCCESS)
        {
            logWarning("Vulkan Renderer: vkQueueSubmit failed with result ", submitResult);
        }

        // ---------------------------------------------------------------------------------
        // Present

        const auto presentInfo = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_pImpl->vkRenderCompleteSemaphores[swapChainImageIndex],      // Wait for the command buffer to finish executing
            .swapchainCount = 1,
            .pSwapchains = &m_pImpl->vkSwapChain,
            .pImageIndices = &swapChainImageIndex,                                              // Which image to draw onto
            .pResults = nullptr                                                                 // (Optional) If using multiple swapchains, the success of each present will be stored in this array as opposed to the singular result from the upcoming call.
        };

        const auto presentResult = vkQueuePresentKHR(m_pImpl->vkGraphicsQueue, &presentInfo);

        if (presentResult != VK_SUCCESS)
        {
            logWarning("Vulkan Renderer: vkQueuePresentKHR failed with result ", presentResult);
        }

        m_pImpl->frame++;
    }

    /// <summary>
    /// Checks if the render fence is open. If not, we are still rendering the last frame and need to wait.
    /// </summary>
    /// <returns></returns>
    bool Renderer::isRenderReady() const
    {
        VkResult fenceResult = vkGetFenceStatus(m_pImpl->vkDevice, m_pImpl->vkRenderFences[getFrameIndex()]);

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
    /// <param name="timeoutNs"></param>
    /// <param name="frameIndex"></param>
    /// <param name="imageIndex"></param>
    /// <returns></returns>
    bool Renderer::acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex)
    {
        const auto result = vkAcquireNextImageKHR(
            m_pImpl->vkDevice,
            m_pImpl->vkSwapChain,
            timeoutNs,
            m_pImpl->vkPresentCompleteSemaphores[frameIndex],
            VK_NULL_HANDLE,
            imageIndex);

        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || m_pImpl->wasResized)
        {
            recreateSwapchain();
            return false;
        }
        else
        {
            return (result == VK_SUCCESS);
        }
    }

    /// <summary>
    /// Records all command buffers and reets the render fence.
    /// </summary>
    /// <param name="pCommandBuffers"></param>
    /// <param name="numCommandBuffers"></param>
    /// <param name="swapChainImageIndex"></param>
    void Renderer::recordCommandBuffers(CommandBuffer* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex)
    {
        for (uint32_t i = 0; i < numCommandBuffers; ++i)
        {
            renderCommandBuffer(&pCommandBuffers[i], swapChainImageIndex);  // Record our render commands
        }

        vkResetFences(m_pImpl->vkDevice, 1, &m_pImpl->vkRenderFences[getFrameIndex()]);     // Reset our draw frame fence
    }

    /// <summary>
    /// Renders the specified command buffer onto the specified image.
    /// </summary>
    /// <param name="pCommandBuffer"></param>
    /// <param name="imageIndex"></param>
    void Renderer::renderCommandBuffer(CommandBuffer* pCommandBuffer, uint32_t imageIndex)
    {
        pCommandBuffer->begin(m_pImpl->frame);

        // We are perfmorming dynamic rendering, so need to specify the image layout we are writing to.
        // Transition from undefined to color
        pCommandBuffer->cmdTransitionImageLayout(
            m_pImpl->vkSwapChainImages[imageIndex],
            VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,                   // From Layout
            VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,    // To Layout
            {},                                                         // Source Access Mask
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                     // Dest Access Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,            // Source Stage Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT             // Dest Stage Mask
        );

        const auto attachmentInfo = VkRenderingAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_pImpl->vkSwapChainImageViews[imageIndex],
            .imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = {{ 0.05f, 0.05f, 0.075f, 1.0f }}
        };

        const auto renderingInfo = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = {0, 0},
                .extent = m_pImpl->vkSwapChainExtent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        const auto viewport = VkViewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_pImpl->vkSwapChainExtent.width),
            .height = static_cast<float>(m_pImpl->vkSwapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        const auto scissor = VkRect2D{
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = {
                .width = m_pImpl->vkSwapChainExtent.width,
                .height = m_pImpl->vkSwapChainExtent.height
            }
        };

        const VkCommandBuffer vkCommandBuffer = pCommandBuffer->getCurrentCommandBuffer(m_pImpl->frame);

        vkCmdBeginRendering(vkCommandBuffer, &renderingInfo);

        vkCmdBindPipeline(vkCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pImpl->vkPipeline);
        vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
        vkCmdDraw(vkCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(vkCommandBuffer);

        // Transition from color to present
        pCommandBuffer->cmdTransitionImageLayout(
            m_pImpl->vkSwapChainImages[imageIndex],
            VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,    // From Layout
            VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,             // To Layout
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                     // Source Access Mask
            {},                                                         // Dest Access Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,            // Source Stage Mask
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                      // Dest Stage Mask
        );

        pCommandBuffer->end();
    }
}