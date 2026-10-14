#ifndef LITL_PLATFORM_CONFIGURATION_H__
#define LITL_PLATFORM_CONFIGURATION_H__

#include <cstdint>
#include "litl-platform/platformTypes.hpp"

namespace litl
{
    struct PlatformConfiguration
    {
        static constexpr uint32_t MinWindowInitialDimensions = 512u;

        /// <summary>
        /// The library used by the platform backend.
        /// </summary>
        PlatformType platformType = PlatformType::Glfw;

        /// <summary>
        /// The name displayed in the application title bar and other areas.
        /// </summary>
        const char* title{ "LITL Engine" };

        /// <summary>
        /// Initial width of the application window.
        /// </summary>
        uint32_t windowWidth{ 1024u };

        /// <summary>
        /// Initial height of the application window.
        /// </summary>
        uint32_t windowHeight{ 1024u };

        // ... todo windowed vs windowed borderless vs fullscreen ...

        void sanitize() noexcept;
    };
}

#endif