#include "litl-core/math.hpp"
#include "litl-renderer/rendererConfiguration.hpp"

namespace litl
{
    void RendererConfiguration::sanitize() noexcept
    {
        framesInFlight = clamp(framesInFlight, 1ul, 8ul);
    }
}