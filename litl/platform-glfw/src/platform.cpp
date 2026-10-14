#include "litl-platform-glfw/platform.hpp"
#include "litl-platform-glfw/window.hpp"

namespace litl
{
    struct PlatformContext
    {

    };

    Platform* createGlfwPlatform() noexcept
    {
        return nullptr;
    }

    namespace glfw
    {
        bool initialize(PlatformContext* context) noexcept
        {
            return false;
        }

        bool shutdown(PlatformContext* context) noexcept
        {
            return false;
        }

        Window* createWindow(PlatformContext* context) noexcept
        {
            return false;
        }
    }
}