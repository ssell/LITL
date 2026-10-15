#include "litl-platform-glfw/platform.hpp"
#include "litl-platform-glfw/window.hpp"
#include "litl-platform-glfw/glfw.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    struct PlatformContext
    {
        bool initialized{ false };
        Window* window{ nullptr };
    };

    PlatformContext& getGlfwPlatformContext() noexcept
    {
        static PlatformContext context{};
        return context;
    }

    Platform& getGlfwPlatform() noexcept
    {
        static Platform platform{};
        return platform;
    }

    Platform* createGlfwPlatform() noexcept
    {
        Platform& platform = getGlfwPlatform();
        PlatformContext& context = getGlfwPlatformContext();

        if (!platform.setup(&GlfwPlatformOps, &context))
        {
            logError("Failed to setup GLFW platform.");
            return nullptr;
        }

        return &platform;
    }

    namespace glfw
    {
        bool initialize(PlatformContext* context) noexcept
        {
            logInfo("Initializing GLFW Platform");

            LITL_ASSERT_MSG(context != nullptr, "Attempted to initialize with null platform.", false);

            const auto initResult = glfwInit();

            if (initResult != GLFW_TRUE)
            {
                logError("GLFW initialization failed with code ", static_cast<uint32_t>(initResult));
                return false;
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            context->initialized = true;

            return true;
        }

        bool shutdown(PlatformContext* context) noexcept
        {
            LITL_FATAL_ASSERT(context != nullptr);

            if (context->window != nullptr)
            {
                // Note: we do not delete window here as it is stack allocated.
                context->window->destroy();
                context->window = nullptr;
            }

            if (context->initialized)
            {
                glfwTerminate();
                context->initialized = false;
            }

            // context is deleted automatically by the service provider singleton which this is injected as

            return true;
        }

        Window* createWindow(PlatformContext* context) noexcept
        {
            if (!context->initialized)
            {
                logError("Requested to create GLFW window when the GLFW subsystem is not yet initialized.");
                return nullptr;
            }

            if (context->window != nullptr)
            {
                logError("Requested to create GLFW window when a window already exists. LITL currently supports only a single window.");
                return nullptr;
            }

            context->window = createGlfwWindow();
            return context->window;
        }

        Window* getWindow(PlatformContext* context) noexcept
        {
            return context->window;
        }
    }
}