#ifndef LITL_IMPORT_RESULT_H__
#define LITL_IMPORT_RESULT_H__

#include <cstdint>
#include <string>

namespace litl::import
{
    enum class ErrorType
    {
        Unknown = 0u,
        ImporterNotImplemented = 1u
    };

    struct Result
    {
        bool success{ false };
        ErrorType error{ ErrorType::Unknown };
        std::string message;
    };
}

#endif