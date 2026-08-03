#include "litl-core/math.hpp"
#include "litl-engine/config.hpp"

namespace litl
{
    void Configuration::sanitize() noexcept
    {
        if (!between(engineSettings.framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps))
        {
            logWarning("Engine settings 'framesPerSecond' is outside of acceptable range of [", EngineConfiguration::MinFixedFps, ", ", EngineConfiguration::MaxFixedFps, "]. Clamping.");
            engineSettings.framesPerSecond = clamp(engineSettings.framesPerSecond, EngineConfiguration::MinFixedFps, EngineConfiguration::MaxFixedFps);
        }

        if (!between(engineSettings.ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond))
        {
            logWarning("Engine settings 'ticksPerSecond' is outside of acceptable range of [", EngineConfiguration::MinTicksPerSecond, ", ", EngineConfiguration::MaxTicksPerSecond, "]. Clamping.");
            engineSettings.ticksPerSecond = clamp(engineSettings.ticksPerSecond, EngineConfiguration::MinTicksPerSecond, EngineConfiguration::MaxTicksPerSecond);
        }

        if (!between(engineSettings.taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount))
        {
            logWarning("Engine settings 'taskThreadCount' is outside of acceptable range of [", EngineConfiguration::MinTaskThreadCount, ", ", EngineConfiguration::MaxTaskThreadCount, "]. Clamping.");
            engineSettings.taskThreadCount = clamp(engineSettings.taskThreadCount, EngineConfiguration::MinTaskThreadCount, EngineConfiguration::MaxTaskThreadCount);
        }

        rendererSettings.sanitize();
    }

    void Configuration::set(Configuration& other) noexcept
    {
        engineSettings = other.engineSettings;
        rendererSettings = other.rendererSettings;
        sceneSettings = other.sceneSettings;

        // ...

        sanitize();
    }
}