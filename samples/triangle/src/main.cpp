#include "litl-engine/engine.hpp"
#include "litl-core/logging/logging.hpp"

int main()
{
    LITL::Engine::Engine engine({});

    LITL::Core::Logger::log(LITL::Core::LogLevel::Info, "Starting Triangle Sample");

    if (!engine.openWindow("LITL - Triangle Sample", 1024, 1024))
    {
        LITL::Core::Logger::log(LITL::Core::LogLevel::Critical, "Failed to open window!");
        return 1;
    }

    while (engine.shouldRun())
    {

    }

    return 0;
}