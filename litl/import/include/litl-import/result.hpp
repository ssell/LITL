#ifndef LITL_IMPORT_RESULT_H__
#define LITL_IMPORT_RESULT_H__

#include <cstdint>
#include <string>
#include <string_view>

namespace litl::import
{
    enum class ErrorType
    {
        None = 0u,
        ImporterNotImplemented = 1u,
        ExporterNotImplemented = 2u,
        NoImporterForSourceExtension = 3u,
        NoExporterForImportedDataType = 4u,
        SourceFileDoesNotExist = 5u,
        FailedToReadSourceFile = 6u,
        EmptySourceFile = 7u,
        ImportedDataTypeMismatch = 8u,
        ImportedDataNull = 9u,
        ImporterFailed = 10u,
        ImporterEmptyResult = 11u
    };

    struct Result
    {
        bool success{ false };
        ErrorType error{ ErrorType::None };
        std::string message;

        [[nodiscard]] static Result Success() noexcept
        {
            return Result{
                .success = true,
                .error = ErrorType::None,
                .message = "Import/export successful."
            };
        }

        static Result Error(ErrorType error) noexcept
        {
            return Error(error, "File import error.");
        }

        [[nodiscard]] static Result Error(ErrorType error, std::string_view message) noexcept
        {
            return Result{
                .success = false,
                .error = error,
                .message = std::string(message)
            };
        }
    };
}

#endif