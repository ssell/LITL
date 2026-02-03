#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "litl-core/logging/logging.hpp"
#include "litl-renderer-vulkan/renderer.hpp"
#include "litl-renderer-vulkan/renderContext.hpp"

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
}