#ifndef LITL_ENGINE_CONFIG_H__
#define LITL_ENGINE_CONFIG_H__

#include <cstdint>

#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-engine/scene/sceneConfiguration.hpp"

namespace litl
{
    class Engine;

    struct EngineConfiguration
    {
        static constexpr uint32_t MinFixedFps = 1u;
        static constexpr uint32_t MaxFixedFps = 1000u;
        static constexpr uint32_t MinTicksPerSecond = 1u;
        static constexpr uint32_t MaxTicksPerSecond = 1000u;
        static constexpr uint32_t MinTaskThreadCount = 1u;
        static constexpr uint32_t MaxTaskThreadCount = 10u;

        /// <summary>
        /// The name displayed in the application title bar and other areas.
        /// </summary>
        const char* applicationName{ "LITL Engine" };

        /// <summary>
        /// Initial width of the application window.
        /// </summary>
        uint32_t windowWidth{ 1024u };

        /// <summary>
        /// Initial height of the application window.
        /// </summary>
        uint32_t windowHeight{ 1024u };

        // ... todo windowed vs windowed borderless vs fullscreen ...

        /// <summary>
        /// The maximum number of frames rendered per second. On the range [MinFixedFps, MaxFixedFps].
        /// </summary>
        uint32_t framesPerSecond{ 120u };

        /// <summary>
        /// The number of fixed update ticks per second. On the range [MinTicksPerSecond, MaxTicksPerSecond].
        /// </summary>
        uint32_t ticksPerSecond{ 50u };

        /// <summary>
        /// The number of threads available in the Task thread pool. Must be on the range [MinTaskThreadCount, MaxTaskThreadCount].
        /// </summary>
        uint32_t taskThreadCount{ 2u };
    };

    struct Configuration
    {
        /// <summary>
        /// General engine settings.
        /// </summary>
        EngineConfiguration engineSettings{};

        /// <summary>
        /// Renderer specific settings.
        /// </summary>
        RendererConfiguration rendererSettings{};

        /// <summary>
        /// Settings for the initial scene.
        /// </summary>
        SceneConfiguration sceneSettings{};

    private:

        friend class Engine;
        void set(Configuration& other) noexcept;
        void sanitize() noexcept;
    };
}

#endif