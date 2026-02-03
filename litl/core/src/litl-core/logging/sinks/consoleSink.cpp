#include <iostream>

#include "litl-core/logging/sinks/consoleSink.hpp"

namespace LITL::Core
{
    void ConsoleLoggingSink::processMessage(std::string_view message)
    {
        std::cout << message << std::endl;
    }
}