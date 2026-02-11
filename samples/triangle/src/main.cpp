#include "litl-engine/engine.hpp"
#include "litl-core/logging/logging.hpp"

int main()
{
    LITL::Engine::Engine engine({});
    LITL::logInfo("Starting Triangle Sample");

    if (!engine.openWindow("LITL - Triangle Sample", 1024, 1024))
    {
        return 1;
    }

    while (engine.shouldRun())
    {
        engine.run();
    }

    return 0;
}