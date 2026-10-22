#ifndef LITL_PLATFORM_CONFIGURATION_H__
#define LITL_PLATFORM_CONFIGURATION_H__

#include <cstdint>

#include "litl-platform/platformTypes.hpp"
#include "litl-platform/window/windowConfiguration.hpp"

namespace litl
{
    struct PlatformConfiguration
    {
        /// <summary>
        /// The library used by the platform backend.
        /// </summary>
        PlatformType platformType = PlatformType::Glfw;

        /// <summary>
        /// Window-specific settings.
        /// </summary>
        WindowConfiguration window{};

        void sanitize() noexcept;
    };
}

#endif