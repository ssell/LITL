#ifndef LITL_ENGINE_CONFIG_H__
#define LITL_ENGINE_CONFIG_H__

#include <cstdint>
#include "litl-renderer/rendererConfiguration.hpp"

namespace litl
{
    class Engine;

    struct EngineConfiguration
    {
        /// <summary>
        /// The name displayed in the application title bar and other areas.
        /// </summary>
        const char* applicationName{ "LITL Engine" };

        /// <summary>
        /// Initial width of the application window.
        /// </summary>
        uint32_t windowWidth{ 1024 };

        /// <summary>
        /// Initial height of the application window.
        /// </summary>
        uint32_t windowHeight{ 1024 };

        // ... todo windowed vs windowed borderless vs fullscreen ...

        /// <summary>
        /// The maximum number of frames rendered per second. On the range [1, 1000].
        /// </summary>
        uint32_t framesPerSecond{ 120 };

        /// <summary>
        /// The number of fixed update ticks per second. On the range [1, 1000].
        /// </summary>
        uint32_t ticksPerSecond{ 50 };
    };

    struct Configuration
    {
        EngineConfiguration engineSettings;
        RendererConfiguration rendererSettings;

    private:

        friend class Engine;
        void set(Configuration& other);
        void sanitize() noexcept;
    };
}

#endif