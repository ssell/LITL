#include "litl-core/math/math.hpp"
#include "litl-engine/config.hpp"

namespace LITL::Engine
{
    void Configuration::sanitize() noexcept
    {
        engineSettings.framesPerSecond = Math::clamp(engineSettings.framesPerSecond, 1ul, 1000ul);
        engineSettings.ticksPerSecond = Math::clamp(engineSettings.ticksPerSecond, 1ul, 1000ul);

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