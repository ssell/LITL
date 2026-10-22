#include "litl-platform/window/windowConfiguration.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    void WindowConfiguration::sanitize() noexcept
    {
        if (title == nullptr)
        {
            logWarning("Platform settings 'title' is NULL. Setting to 'LITL Engine'.");
            title = "LITL Engine";
        }

        if (width < MinWindowInitialDimensions)
        {
            logWarning("Platform settings 'windowWidth' is under the minimum dimensions of ", MinWindowInitialDimensions, ". Setting to the minimum value.");
            width = MinWindowInitialDimensions;
        }

        if (height < MinWindowInitialDimensions)
        {
            logWarning("Platform settings 'height' is under the minimum dimensions of ", MinWindowInitialDimensions, ". Setting to the minimum value.");
            height = MinWindowInitialDimensions;
        }
    }
}