#include "litl-engine/engine.hpp"
#include <iostream>

int main()
{
    LITL::Engine::Engine engine(LITL::Renderer::RendererBackendType::Vulkan);

    if (!engine.openWindow("LITL - Triangle Sample", 1024, 1024))
    {
        std::cout << "Failed creating window!" << std::endl;
    }

    int a = 0;
    std::cin >> a;
}