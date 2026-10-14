#ifndef LITL_ENGINE_PLATFORM_FACTORY_H__
#define LITL_ENGINE_PLATFORM_FACTORY_H__

#include "litl-platform/platformTypes.hpp"

namespace litl
{
    class ServiceProvider;

    /// <summary>
    /// Creates a new Platform with the requested backend.
    /// </summary>
    [[nodiscard]] bool injectPlatform(ServiceProvider& serviceProvider, PlatformType platformType) noexcept;
}

#endif