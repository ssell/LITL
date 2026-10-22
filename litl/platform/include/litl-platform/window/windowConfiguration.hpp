#ifndef LITL_PLATFORM_WINDOW_CONFIGURATION_H__
#define LITL_PLATFORM_WINDOW_CONFIGURATION_H__

#include <cstdint>

namespace litl
{
    struct WindowConfiguration
    {
        static constexpr uint32_t MinWindowInitialDimensions = 512u;

        /// <summary>
        /// The name displayed in the application title bar and other areas.
        /// </summary>
        const char* title{ "LITL Engine" };

        /// <summary>
        /// Initial width of the application window.
        /// </summary>
        uint32_t width{ 1024u };

        /// <summary>
        /// Initial height of the application window.
        /// </summary>
        uint32_t height{ 1024u };

        // ... todo windowed vs windowed borderless vs fullscreen ...

        void sanitize() noexcept;
    };
}

#endif