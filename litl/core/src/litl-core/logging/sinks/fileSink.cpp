#include <fstream>
#include "litl-core/logging/sinks/fileSink.hpp"

namespace LITL::Core
{
    FileLoggingSink::FileLoggingSink(char const* logName)
        : m_logFileName(logName)
    {
        std::ofstream fileStream(m_logFileName, std::ios::out | std::ios::trunc);       // erase the file

        if (fileStream.is_open())
        {
            fileStream.close();
        }
    }

    void FileLoggingSink::processMessage(std::string const& message)
    {
        std::ofstream fileStream(m_logFileName, std::ios::out | std::ios::app);

        if (fileStream.is_open())
        {
            fileStream << message << std::endl;
            fileStream.close();
        }
    }
}