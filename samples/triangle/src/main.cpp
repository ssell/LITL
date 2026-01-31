#include "litl-engine/engine.hpp"
#include <iostream>

int main()
{
    LITL::Engine::Engine engine({});

    if (!engine.openWindow("LITL - Triangle Sample", 1024, 1024))
    {
        std::cout << "Failed creating window!" << std::endl;
        return 1;
    }

    while (engine.shouldRun())
    {

    }

    return 0;
}