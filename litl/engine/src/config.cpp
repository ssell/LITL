#include "litl-core/math.hpp"
#include "litl-engine/config.hpp"

namespace litl
{
    void Configuration::sanitize() noexcept
    {
        engineSettings.framesPerSecond = clamp(engineSettings.framesPerSecond, 1ul, 1000ul);
        engineSettings.ticksPerSecond = clamp(engineSettings.ticksPerSecond, 1ul, 1000ul);

        rendererSettings.sanitize();
    }

    void Configuration::set(Configuration& other) noexcept
    {
        engineSettings = other.engineSettings;
        rendererSettings = other.rendererSettings;
        // ...

        sanitize();
    }
}