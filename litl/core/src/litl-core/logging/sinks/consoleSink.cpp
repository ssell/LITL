#include <iostream>
#include "litl-core/logging/sinks/consoleSink.hpp"

namespace LITL::Core
{
    void ConsoleLoggingSink::processMessage(std::string const& message)
    {
        std::cout << message << std::endl;
    }
}