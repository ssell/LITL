#include "litl-engine/startup.hpp"

using namespace litl;

int main()
{
    Engine engine{};

    engine.setup(
        { .engineSettings {.applicationName = "LITL - Boids Sample" } },
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );

    engine.start();
    
    return 0;
}