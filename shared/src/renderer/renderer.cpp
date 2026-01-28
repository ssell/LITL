#include <vulkan/vulkan.hpp>
#include <cstring>
#include <vector>
#include <set>
#include <string>

#include "glfw.hpp"
#include "math/mathCommon.hpp"
#include "renderer/common.inl"
#include "renderer/renderer.hpp"
#include "renderer/queueFamily.hpp"
#include "renderer/swapChainSupport.hpp"
#include "renderer/shader/vertexShader.hpp"
#include "renderer/shader/geometryShader.hpp"
#include "renderer/shader/fragmentShader.hpp"

namespace LITL
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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // -------------------------------------------------------------------------------------
    // Renderer Creation
    // -------------------------------------------------------------------------------------

    Renderer::Renderer()
        : m_pContext(new RenderContext{})
    {

    }

    Renderer::~Renderer()
    {
        cleanup();
    }

    /// <summary>
    /// Prepares the renderer and all underlying devices, queues, surfaces, etc.
    /// </summary>
    /// <param name="windowWidth"></param>
    /// <param name="windowHeight"></param>
    /// <param name="applicationName"></param>
    /// <returns></returns>
    bool Renderer::initialize(uint32_t windowWidth, uint32_t windowHeight, const void* application, const char* applicationName) const
    {
        m_pContext->window = createGlfwWindow(windowWidth, windowHeight, applicationName, const_cast<void*>(application));

        if (!m_pContext->window)
        {
            return false;
        }

        return createInstance(applicationName) &&
               createWindowSurface() &&
               selectPhysicalDevice() &&
               createLogicalDevice() && 
               createSwapChain() &&
               createCommandPool() &&
               createSyncObjects();
    }

    /// <summary>
    /// Must be invoked when the window is resized so that the swapchain (and it's dependencies) may be recreated accordingly.
    /// </summary>
    /// <returns></returns>
    bool Renderer::onResize(int width, int height) const
    {
        if (m_pContext->device == VK_NULL_HANDLE)
        {
            return false;
        }

        // While the resize is typically caught in acquireSwapChainIndex, this is not guaranteed on 100% of drivers.
        m_pContext->wasResized = true;

        return true;
    }

    /// <summary>
    /// Returns true as long as we have not received an exit signal from the window.
    /// </summary>
    /// <returns></returns>
    bool Renderer::shouldRun() const
    {
        glfwPollEvents();
        return !glfwWindowShouldClose(static_cast<GLFWwindow*>(m_pContext->window));
    }

    // -------------------------------------------------------------------------------------
    // Cleanup
    // -------------------------------------------------------------------------------------

    void Renderer::cleanup() const
    {
        const auto frameIndex = m_pContext->getFrameIndex();

        for (auto i = 0; i < m_pContext->presentCompleteSemaphores.size(); ++i)
        {
            if (m_pContext->presentCompleteSemaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(m_pContext->device, m_pContext->presentCompleteSemaphores[i], nullptr);
            }

            m_pContext->presentCompleteSemaphores.clear();
        }

        for (auto i = 0; i < m_pContext->renderCompleteSemaphores.size(); ++i)
        {
            if (m_pContext->renderCompleteSemaphores[i] != VK_NULL_HANDLE)
            {
                vkDestroySemaphore(m_pContext->device, m_pContext->renderCompleteSemaphores[i], nullptr);
            }

            m_pContext->renderCompleteSemaphores.clear();
        }

        for (auto i = 0; i < m_pContext->renderFences.size(); ++i)
        {
            if (m_pContext->renderFences[i] != VK_NULL_HANDLE)
            {
                vkDestroyFence(m_pContext->device, m_pContext->renderFences[i], nullptr);
            }

            m_pContext->renderFences.clear();
        }

        if (m_pContext->commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_pContext->device, m_pContext->commandPool, nullptr);
        }

        if (m_pContext->pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_pContext->device, m_pContext->pipeline, nullptr);
        }

        if (m_pContext->pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_pContext->device, m_pContext->pipelineLayout, nullptr);
        }

        cleanupSwapchain();

        if (m_pContext->surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(m_pContext->vkInstance, m_pContext->surface, nullptr);
        }

        if (m_pContext->device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(m_pContext->device, nullptr);
        }

        if (m_pContext->vkInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_pContext->vkInstance, nullptr);
        }

        if (m_pContext->window != nullptr)
        {
            closeGlfwWindow(static_cast<GLFWwindow*>(m_pContext->window));
        }

        delete m_pContext;
    }

    void Renderer::cleanupSwapchain() const
    {
        if (!m_pContext->swapChainImageViews.empty())
        {
            // Note: only have to destroy the views since we explicitly made them, and not the underlying images.
            // Those will be destroyed alongside the swap chain itself since it made them.
            for (auto imageView : m_pContext->swapChainImageViews)
            {
                vkDestroyImageView(m_pContext->device, imageView, nullptr);
            }
        }

        if (m_pContext->swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(m_pContext->device, m_pContext->swapChain, nullptr);
        }
    }

    void Renderer::recreateSwapchain() const
    {
        // Wait for our resources to be unused.
        vkDeviceWaitIdle(m_pContext->device);

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
    bool Renderer::createInstance(const char* applicationName) const
    {
        // See: https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/00_Setup/01_Instance.html
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = applicationName;
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

        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_pContext->vkInstance);

        if (result != VK_SUCCESS)
        {
            // todo log out
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
        VkResult result = glfwCreateWindowSurface(m_pContext->vkInstance, static_cast<GLFWwindow*>(m_pContext->window), nullptr, &m_pContext->surface);

        if (result != VK_SUCCESS)
        {
            // todo log out
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
        vkEnumeratePhysicalDevices(m_pContext->vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            return false;
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(m_pContext->vkInstance, &deviceCount, availableDevices.data());

        for (auto device : availableDevices)
        {
            auto queueFamilies = findQueueFamilies(device, m_pContext->surface);

            if (isPhysicalDeviceSuitable(device) && queueFamilies.hasAll())
            {
                auto swapChainSupport = SwapChainSupport::querySwapChainSupport(device, m_pContext->surface);

                if (swapChainSupport.isSwapChainSufficient())
                {
                    m_pContext->physicalDevice = device;
                    break;
                }
            }
        }

        return (m_pContext->physicalDevice != VK_NULL_HANDLE);
    }

    /// <summary>
    /// Creates the logical device and command queues.
    /// </summary>
    /// <returns></returns>
    bool Renderer::createLogicalDevice() const
    {
        auto queueFamilies = findQueueFamilies(m_pContext->physicalDevice, m_pContext->surface);

        m_pContext->graphicsQueueIndex = queueFamilies.getGraphicsIndex();
        m_pContext->presentQueueIndex = queueFamilies.getPresentIndex();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_pContext->graphicsQueueIndex, m_pContext->presentQueueIndex };
        float queuePriorities[1] = { 1.0f };

        for (auto queueFamilyIndex : uniqueQueueFamilies)
        {
            queueCreateInfos.push_back(VkDeviceQueueCreateInfo {
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

        const auto deviceCreateInfo = VkDeviceCreateInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &physicalDeviceFeatures,
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledExtensionCount = static_cast<uint32_t>(RequiredDeviceExtensions.size()),
            .ppEnabledExtensionNames = RequiredDeviceExtensions.data()
        };

        const VkResult result = vkCreateDevice(m_pContext->physicalDevice, &deviceCreateInfo, nullptr, &m_pContext->device);

        if (result != VK_SUCCESS)
        {
            // todo log out
            return false;
        }

        vkGetDeviceQueue(m_pContext->device, queueFamilies.getGraphicsIndex(), 0, &m_pContext->graphicsQueue);
        vkGetDeviceQueue(m_pContext->device, queueFamilies.getPresentIndex(), 0, &m_pContext->presentQueue);

        if ((m_pContext->graphicsQueue == VK_NULL_HANDLE) || (m_pContext->presentQueue == VK_NULL_HANDLE))
        {
            // todo log out
            return false;
        }

        return true;
    }

    bool Renderer::createSwapChain() const
    {
        m_pContext->wasResized = false;

        int32_t frameBufferWidth = 0;
        int32_t frameBufferHeight = 0;

        glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_pContext->window), &frameBufferWidth, &frameBufferHeight);

        auto swapChainSupport = SwapChainSupport::querySwapChainSupport(m_pContext->physicalDevice, m_pContext->surface);
        auto surfaceFormat = swapChainSupport.chooseSwapSurfaceFormat();
        auto presentMode = swapChainSupport.chooseSwapPresentMode();
        auto imageExtent = swapChainSupport.chooseSwapExtent(frameBufferWidth, frameBufferHeight);
        auto imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0)        // == 0 means no limit
        {
            imageCount = minimum(imageCount, swapChainSupport.capabilities.maxImageCount);
        }

        VkSwapchainCreateInfoKHR createSwapChainInfo{};
        createSwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createSwapChainInfo.surface = m_pContext->surface;
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

        auto queueIndices = findQueueFamilies(m_pContext->physicalDevice, m_pContext->surface);
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

        VkResult result = vkCreateSwapchainKHR(m_pContext->device, &createSwapChainInfo, nullptr, &m_pContext->swapChain);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        m_pContext->swapChainImageFormat = surfaceFormat.format;
        m_pContext->swapChainExtent = imageExtent;

        vkGetSwapchainImagesKHR(m_pContext->device, m_pContext->swapChain, &imageCount, nullptr);
        m_pContext->swapChainImages.resize(imageCount);
        m_pContext->swapChainImageViews.resize(imageCount);
        vkGetSwapchainImagesKHR(m_pContext->device, m_pContext->swapChain, &imageCount, m_pContext->swapChainImages.data());

        for (uint32_t i = 0; i < imageCount; ++i)
        {
            VkImageViewCreateInfo createImageViewInfo{};
            createImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createImageViewInfo.image = m_pContext->swapChainImages[i];
            createImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createImageViewInfo.format = m_pContext->swapChainImageFormat;
            createImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createImageViewInfo.subresourceRange.baseMipLevel = 0;
            createImageViewInfo.subresourceRange.levelCount = 1;
            createImageViewInfo.subresourceRange.baseArrayLayer = 0;
            createImageViewInfo.subresourceRange.layerCount = 1;

            result = vkCreateImageView(m_pContext->device, &createImageViewInfo, nullptr, &m_pContext->swapChainImageViews[i]);

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

        const auto commandPoolInfo = VkCommandPoolCreateInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,       // Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
            .queueFamilyIndex = m_pContext->graphicsQueueIndex
        };

        const VkResult result = vkCreateCommandPool(m_pContext->device, &commandPoolInfo, nullptr, &m_pContext->commandPool);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        return true;
    }

    bool Renderer::createSyncObjects() const
    {
        if (m_pContext->presentCompleteSemaphores.size() > 0)
        {
            return false;
        }

        const auto presentSemaphoreInfo = VkSemaphoreCreateInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        const auto renderSemaphoreInfo = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        const auto renderFenceInfo = VkFenceCreateInfo { 
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT 
        };

        // Note that renderCompleteSemaphores are tied to swapchain image count (which is the device minimum + 1 (so 3 on this machine))
        // And that presentCompleteSemaphores and renderFences are tied to FRAMES_IN_FLIGHT (which is currently 2)

        m_pContext->renderCompleteSemaphores.resize(m_pContext->swapChainImages.size(), VK_NULL_HANDLE);
        m_pContext->presentCompleteSemaphores.resize(FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
        m_pContext->renderFences.resize(FRAMES_IN_FLIGHT, VK_NULL_HANDLE);

        for (size_t i = 0; i < m_pContext->swapChainImages.size(); ++i)
        {
            const auto renderCompleteSemaphoreResult = vkCreateSemaphore(m_pContext->device, &renderSemaphoreInfo, nullptr, &m_pContext->renderCompleteSemaphores[i]);

            if (renderCompleteSemaphoreResult != VK_SUCCESS)
            {
                // todo log error
                return false;
            }
        }

        for (size_t i = 0; i < FRAMES_IN_FLIGHT; ++i)
        {
            const auto presentCompleteSemaphoreResult = vkCreateSemaphore(m_pContext->device, &presentSemaphoreInfo, nullptr, &m_pContext->presentCompleteSemaphores[i]);
            const auto renderFenceResult = vkCreateFence(m_pContext->device, &renderFenceInfo, nullptr, &m_pContext->renderFences[i]);

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

    Shader* Renderer::createShader(ShaderStage shaderStage) const noexcept
    {
        switch (shaderStage)
        {
        case ShaderStage::Vertex:
            return new VertexShader(m_pContext);

        case ShaderStage::PreTessellation:
            return new PreTessellationShader(m_pContext);

        case ShaderStage::PostTessellation:
            return new PostTessellationShader(m_pContext);

        case ShaderStage::Geometry:
            return new GeometryShader(m_pContext);

        case ShaderStage::Fragment:
            return new FragmentShader(m_pContext);

        case ShaderStage::Compute:
            return new ComputeShader(m_pContext);

        default:
            return nullptr;
        }
    }

    ShaderProgram* Renderer::createShaderProgram() const noexcept
    {
        return new ShaderProgram(m_pContext);
    }

    CommandBuffer* Renderer::createCommandBuffer() const noexcept
    {
        auto commandBuffer = new CommandBuffer(m_pContext);
        commandBuffer->build();

        return commandBuffer;
    }

    // -------------------------------------------------------------------------------------
    // Pipeline Creation
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Must be invoked after a shader is created/loaded so that it is included in the graphics pipeline.
    /// I have a feeling this needs to be rearchitectured ...
    /// </summary>
    /// <param name="shaders"></param>
    /// <param name="shaderProgramCount"></param>
    /// <returns></returns>
    bool Renderer::recreateShaderPipeline(ShaderProgram** shaders, uint32_t shaderProgramCount) const
    {
        // https://docs.vulkan.org/tutorial/latest/03_Drawing_a_triangle/02_Graphics_pipeline_basics/02_Fixed_functions.html

        if (m_pContext->pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(m_pContext->device, m_pContext->pipeline, nullptr);
        }

        if (m_pContext->pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(m_pContext->device, m_pContext->pipelineLayout, nullptr);
        }

        // Not using buffers atm (baked into shader), update when using buffers.
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        // Specify viewport and scissor functionality as dynamic. Will need to be set at draw time.
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        VkResult result = vkCreatePipelineLayout(m_pContext->device, &pipelineLayoutInfo, nullptr, &m_pContext->pipelineLayout);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        std::vector<VkPipelineShaderStageCreateInfo> allShaderStages;

        for (uint32_t i = 0; i < shaderProgramCount; ++i)
        {
            auto vs = shaders[i]->getVertexShader();
            auto pr = shaders[i]->getPreTessellationShader();
            auto po = shaders[i]->getPostTessellationShader();
            auto gs = shaders[i]->getGeometryShader();
            auto fs = shaders[i]->getFragmentShader();

            if (vs->getContext()->shaderModule != VK_NULL_HANDLE)
            {
                allShaderStages.push_back(vs->getContext()->shaderStageInfo);
            }

            if (pr->getContext()->shaderModule != VK_NULL_HANDLE)
            {
                allShaderStages.push_back(pr->getContext()->shaderStageInfo);
            }

            if (po->getContext()->shaderModule != VK_NULL_HANDLE)
            {
                allShaderStages.push_back(po->getContext()->shaderStageInfo);
            }

            if (gs->getContext()->shaderModule != VK_NULL_HANDLE)
            {
                allShaderStages.push_back(gs->getContext()->shaderStageInfo);
            }

            if (fs->getContext()->shaderModule != VK_NULL_HANDLE)
            {
                allShaderStages.push_back(fs->getContext()->shaderStageInfo);
            }
        }

        const auto pipelineRenderingInfo = VkPipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount = 1,
            .pColorAttachmentFormats = &m_pContext->swapChainImageFormat
        };
        
        const auto pipelineInfo = VkGraphicsPipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingInfo,
            .stageCount = static_cast<uint32_t>(allShaderStages.size()),
            .pStages = allShaderStages.data(),
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterizer,
            .pMultisampleState = &multisampling,
            .pDepthStencilState = nullptr,              // Optional
            .pColorBlendState = &colorBlending,
            .pDynamicState = &dynamicState,
            .layout = m_pContext->pipelineLayout,
            .renderPass = nullptr,                      // Not used with dynamic rendering
            .subpass = 0,                               // Not currently used
            .basePipelineHandle = nullptr,              // Optional
            .basePipelineIndex = -1,                    // Optional
        };

        result = vkCreateGraphicsPipelines(m_pContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pContext->pipeline);

        if (result != VK_SUCCESS)
        {
            // todo log error
            return false;
        }

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Rendering
    // -------------------------------------------------------------------------------------

    void Renderer::render(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers) const
    {
        // ---------------------------------------------------------------------------------
        // Wait & Prepare 

        if (!isRenderReady())
        {
            return;
        }

        const auto frameIndex = m_pContext->getFrameIndex();
        uint32_t swapChainImageIndex = 0;
        
        if (!acquireSwapChainIndex(1000000, frameIndex, &swapChainImageIndex))                              // 1000000 ns = 1 ms
        {
            return;
        }

        // ---------------------------------------------------------------------------------
        // Record Commands

        vkResetFences(m_pContext->device, 1, &m_pContext->renderFences[frameIndex]);
        recordCommandBuffers(pCommandBuffers, numCommandBuffers, swapChainImageIndex);

        // ---------------------------------------------------------------------------------
        // Submit Commands

        const auto waitDestinationStageMask = VkPipelineStageFlags(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

        // Only 1 expected command buffer atm. If we move to actually have multiple, reconsider redesigning CommandBuffer for AoS vs SoA.
        const auto vkCommandBuffer = (&pCommandBuffers)[0]->getCommandBufferContext()->getCurrentCommandBuffer(m_pContext); 

        const auto submitInfo = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_pContext->presentCompleteSemaphores[frameIndex],              // Wait for current swapchain image to be done presenting
            .pWaitDstStageMask = &waitDestinationStageMask,                                     // Wait for writing colors to the image until's available
            .commandBufferCount = 1,
            .pCommandBuffers = &vkCommandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_pContext->renderCompleteSemaphores[swapChainImageIndex]     // The semaphore to signal once the command buffer(s) have finished execution.
        };

        const auto submitResult = vkQueueSubmit(m_pContext->graphicsQueue, 1, &submitInfo, m_pContext->renderFences[frameIndex]);      // Submit the command buffer

        // ---------------------------------------------------------------------------------
        // Present

        const auto presentInfo = VkPresentInfoKHR{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_pContext->renderCompleteSemaphores[swapChainImageIndex],      // Wait for the command buffer to finish executing
            .swapchainCount = 1,
            .pSwapchains = &m_pContext->swapChain,
            .pImageIndices = &swapChainImageIndex,                                              // Which image to draw onto
            .pResults = nullptr                                                                 // (Optional) If using multiple swapchains, the success of each present will be stored in this array as opposed to the singular result from the upcoming call.
        };

        const auto presentResult = vkQueuePresentKHR(m_pContext->graphicsQueue, &presentInfo);

        m_pContext->frame++;
    }

    /// <summary>
    /// Checks if the render fence is open. If not, we are still rendering the last frame and need to wait.
    /// </summary>
    /// <returns></returns>
    bool Renderer::isRenderReady() const
    {
        VkResult fenceResult = vkGetFenceStatus(m_pContext->device, m_pContext->renderFences[m_pContext->getFrameIndex()]);

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
    bool Renderer::acquireSwapChainIndex(uint32_t timeoutNs, uint32_t frameIndex, uint32_t* imageIndex) const
    {
        const auto result = vkAcquireNextImageKHR(
            m_pContext->device, 
            m_pContext->swapChain, 
            timeoutNs, 
            m_pContext->presentCompleteSemaphores[frameIndex],
            VK_NULL_HANDLE, 
            imageIndex);

        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR) || m_pContext->wasResized)
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
    void Renderer::recordCommandBuffers(CommandBuffer const* pCommandBuffers, uint32_t numCommandBuffers, uint32_t swapChainImageIndex) const
    {
        for (uint32_t i = 0; i < numCommandBuffers; ++i)
        {
            renderCommandBuffer(&pCommandBuffers[i], swapChainImageIndex);  // Record our render commands
        }

        vkResetFences(m_pContext->device, 1, &m_pContext->renderFences[m_pContext->getFrameIndex()]);     // Reset our draw frame fence
    }

    /// <summary>
    /// Renders the specified command buffer onto the specified image.
    /// </summary>
    /// <param name="pCommandBuffer"></param>
    /// <param name="imageIndex"></param>
    void Renderer::renderCommandBuffer(CommandBuffer const* pCommandBuffer, uint32_t imageIndex) const
    {
        pCommandBuffer->begin(m_pContext);

        // We are perfmorming dynamic rendering, so need to specify the image layout we are writing to.
        // Transition from undefined to color
        pCommandBuffer->commandImageLayoutTransition(
            m_pContext,
            imageIndex,
            VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,                   // From Layout
            VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,    // To Layout
            {},                                                         // Source Access Mask
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                     // Dest Access Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,            // Source Stage Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT             // Dest Stage Mask
        );

        const auto attachmentInfo = VkRenderingAttachmentInfo {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_pContext->swapChainImageViews[imageIndex],
            .imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = VkClearColorValue{0.05f, 0.05f, 0.075f, 1.0f}
        };

        const auto renderingInfo = VkRenderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = {0, 0},
                .extent = m_pContext->swapChainExtent
            },
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &attachmentInfo
        };

        const auto viewport = VkViewport {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(m_pContext->swapChainExtent.width),
            .height = static_cast<float>(m_pContext->swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        const auto scissor = VkRect2D {
            .offset = {
                .x = 0,
                .y = 0
            },
            .extent = {
                .width = m_pContext->swapChainExtent.width,
                .height = m_pContext->swapChainExtent.height
            }
        };

        const VkCommandBuffer vkCommandBuffer = pCommandBuffer->getCommandBufferContext()->getCurrentCommandBuffer(m_pContext);

        vkCmdBeginRendering(vkCommandBuffer, &renderingInfo);

        vkCmdBindPipeline(vkCommandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pContext->pipeline);
        vkCmdSetViewport(vkCommandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(vkCommandBuffer, 0, 1, &scissor);
        vkCmdDraw(vkCommandBuffer, 3, 1, 0, 0);

        vkCmdEndRendering(vkCommandBuffer);

        // Transition from color to present
        pCommandBuffer->commandImageLayoutTransition(
            m_pContext,
            imageIndex,
            VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,    // From Layout
            VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,             // To Layout
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,                     // Source Access Mask
            {},                                                         // Dest Access Mask
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,            // Source Stage Mask
            VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT                      // Dest Stage Mask
        );

        pCommandBuffer->end(m_pContext);
    }
}