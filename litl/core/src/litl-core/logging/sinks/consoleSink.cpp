#include <iostream>
#include "litl-core/logging/sinks/consoleSink.hpp"

namespace litl
{
    void ConsoleLoggingSink::processMessage(std::string const& message)
    {
        std::cout << message << std::endl;
    }
}