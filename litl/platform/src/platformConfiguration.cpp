#include "litl-platform/platformConfiguration.hpp"

namespace litl
{
    void PlatformConfiguration::sanitize() noexcept
    {
        window.sanitize();
        input.sanitize();
    }
}