#include "litl-engine/engine.hpp"


int main()
{
    LITL::Engine::Engine engine({});

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
