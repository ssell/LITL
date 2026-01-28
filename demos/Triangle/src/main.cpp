/**
 * A minimal Vulkan project that draws a triangle to the screen.
 */

#include "triangleProgram.hpp"
#include <iostream>

int main()
{
    LITL::TriangleProgram program{};

    if (!program.initialize())
    {
        std::cout << "Failed to initialize demo program!" << std::endl;
        return 1;
    }

    while (program.shouldRun())
    {
        program.run();
    }

    program.shutdown();

    return 0;
}
