#include "litl-core/math.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/config.hpp"

namespace litl
{
    void Configuration::sanitize() noexcept
    {
        if (!between(engine.framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps))
        {
            logWarning("Engine settings 'framesPerSecond' is outside of acceptable range of [", EngineConfiguration::MinFixedFps, ", ", EngineConfiguration::MaxFixedFps, "]. Clamping.");
            engine.framesPerSecond = clamp(engine.framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps);
        }

        if (!between(engine.ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond))
        {
            logWarning("Engine settings 'ticksPerSecond' is outside of acceptable range of [", EngineConfiguration::MinTicksPerSecond, ", ", EngineConfiguration::MaxTicksPerSecond, "]. Clamping.");
            engine.ticksPerSecond = clamp(engine.ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond);
        }

        if (!between(engine.taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount))
        {
            logWarning("Engine settings 'taskThreadCount' is outside of acceptable range of [", EngineConfiguration::MinTaskThreadCount, ", ", EngineConfiguration::MaxTaskThreadCount, "]. Clamping.");
            engine.taskThreadCount = clamp(engine.taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount);
        }

        platform.sanitize();
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