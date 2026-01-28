/**
 * A minimal template C++ project that runs.
 */

#include "template.hpp"
#include <iostream>

int main()
{
    LITL::TemplateProgram program{};

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
