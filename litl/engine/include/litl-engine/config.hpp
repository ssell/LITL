#ifndef LITL_ENGINE_CONFIG_H__
#define LITL_ENGINE_CONFIG_H__

#include <cstdint>

#include "litl-renderer/rendererConfiguration.hpp"
#include "litl-platform/platformConfiguration.hpp"
#include "litl-engine/scene/sceneConfiguration.hpp"
#include "litl-platform/input/inputConfiguration.hpp"

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

        void sanitize() noexcept;
    };

    struct Configuration
    {
        /// <summary>
        /// General engine settings.
        /// </summary>
        EngineConfiguration engine{};

        /// <summary>
        /// Platform specific settings.
        /// </summary>
        PlatformConfiguration platform{};

        /// <summary>
        /// Renderer specific settings.
        /// </summary>
        RendererConfiguration renderer{};

        /// <summary>
        /// Settings for the initial scene.
        /// </summary>
        SceneConfiguration scene{};

    private:

        friend class Engine;
        void set(Configuration& other) noexcept;
        void sanitize() noexcept;
    };
}

#endif