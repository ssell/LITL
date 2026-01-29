#include "litl/engine/engine.hpp"
#include <iostream>

int main()
{
    const LITL::Engine::Engine engine{};

    if (engine.configureWindow("LITL - Triangle Sample", 1024, 1024))
    {
        std::cout << "Successfully created window!" << std::endl;
    }
    else
    {
        std::cout << "Failed creating window!" << std::endl;
    }

    int a = 0;
    std::cin >> a;
}