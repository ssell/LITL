#include "litl-core/math.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/config.hpp"

namespace litl
{
    void EngineConfiguration::sanitize() noexcept
    {
        if (!between(framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps))
        {
            logWarning("Engine settings 'framesPerSecond' is outside of acceptable range of [", EngineConfiguration::MinFixedFps, ", ", EngineConfiguration::MaxFixedFps, "]. Clamping.");
            framesPerSecond = clamp(framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps);
        }

        if (!between(ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond))
        {
            logWarning("Engine settings 'ticksPerSecond' is outside of acceptable range of [", EngineConfiguration::MinTicksPerSecond, ", ", EngineConfiguration::MaxTicksPerSecond, "]. Clamping.");
            ticksPerSecond = clamp(ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond);
        }

        if (!between(taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount))
        {
            logWarning("Engine settings 'taskThreadCount' is outside of acceptable range of [", EngineConfiguration::MinTaskThreadCount, ", ", EngineConfiguration::MaxTaskThreadCount, "]. Clamping.");
            taskThreadCount = clamp(taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount);
        }
    }

    void Configuration::sanitize() noexcept
    {
        engine.sanitize();
        platform.sanitize();
        input.sanitize();
        renderer.sanitize();
    }

    void Configuration::set(Configuration& other) noexcept
    {
        engine = other.engine;
        platform = other.platform;
        renderer = other.renderer;
        scene = other.scene;

        // ...

        sanitize();
    }
}