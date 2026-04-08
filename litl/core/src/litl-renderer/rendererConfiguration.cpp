#include "litl-core/math.hpp"
#include "litl-renderer/rendererConfiguration.hpp"

namespace LITL::Renderer
{
    void RendererConfiguration::sanitize() noexcept
    {
        framesInFlight = Math::clamp(framesInFlight, 1ul, 8ul);
    }
}