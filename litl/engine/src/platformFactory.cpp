#include "litl-engine/platformFactory.hpp"
#include "litl-platform/platform.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/assert.hpp"

#ifdef LITL_PLATFORM_GLFW
#include "litl-platform-glfw/integration.hpp"
#endif

namespace litl
{
    bool injectPlatform(ServiceProvider& serviceProvider, PlatformType platformType) noexcept
    {
        Platform* platform = nullptr;

        switch (platformType)
        {
#ifdef LITL_PLATFORM_GLFW
        case PlatformType::Glfw:
            platform = createGlfwPlatform();
            break;
#endif

        default:
            logError("Requested to create unsupported platform type '", getPlatformName(platformType), "'");
            return false;
        }

        if (platform == nullptr)
        {
            logError("Failed to create Platform of type '", getPlatformName(platformType), "'");
        }

        if (!platform->initialize())
        {
            logError("Failed to initialize Platform of type '", getPlatformName(platformType), "'");
            return false;
        }

        if (!serviceProvider.setSingleton<Platform, Platform>(platform))
        {
            logError("Failed to inject Platform of type '", getPlatformName(platformType), "' to ServiceProvider");
            
            if (!platform->shutdown())
            {
                logError("Failed to shutdown Platform of type '", getPlatformName(platformType), "' following bad injection.");
            }

            return false;
        }

        return true;
    }
}