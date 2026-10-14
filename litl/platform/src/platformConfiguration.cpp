#include "litl-core/math.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-platform/platformConfiguration.hpp"

namespace litl
{
    void PlatformConfiguration::sanitize() noexcept
    {
        if (title == nullptr)
        {
            logWarning("Platform settings 'title' is NULL. Setting to 'LITL Engine'.");
            title = "LITL Engine";
        }

        if (windowWidth < MinWindowInitialDimensions)
        {
            logWarning("Platform settings 'windowWidth' is under the minimum dimensions of ", MinWindowInitialDimensions, ". Setting to the minimum value.");
            windowWidth = MinWindowInitialDimensions;
        }

        if (windowHeight < MinWindowInitialDimensions)
        {
            logWarning("Platform settings 'windowHeight' is under the minimum dimensions of ", MinWindowInitialDimensions, ". Setting to the minimum value.");
            windowHeight = MinWindowInitialDimensions;
        }
    }
}