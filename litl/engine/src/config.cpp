#include "litl-core/math/math.hpp"
#include "litl-engine/config.hpp"

namespace LITL::Engine
{
    void Configuration::sanitize() noexcept
    {
        simulationSettings.framesPerSecond = Math::clamp(simulationSettings.framesPerSecond, 1ul, 1000ul);
        simulationSettings.ticksPerSecond = Math::clamp(simulationSettings.ticksPerSecond, 1ul, 1000ul);

        rendererSettings.sanitize();
    }
}