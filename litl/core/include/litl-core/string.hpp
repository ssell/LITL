#ifndef LITL_CORE_STRING_H__
#define LITL_CORE_STRING_H__

#include <cctype>
#include <string>
#include <string_view>

namespace litl
{
    /// <summary>
    /// Converts the provided string to lowercase.
    /// </summary>
    [[nodiscard]] std::string toLowercase(std::string_view str) noexcept;

    /// <summary>
    /// Returns a view of the string with the leading whitespace ommitted.
    /// </summary>
    [[nodiscard]] std::string_view trimLeadingWhitespace(std::string_view str) noexcept;

    /// <summary>
    /// Returns true if the specified character is whitespace.
    /// </summary>
    [[nodiscard]] bool isWhitespace(char c) noexcept;

    /// <summary>
    /// Returns the index of the first whitespace character in the string view.
    /// Returns std::string_view::npos if there are no whitespace characters.
    /// </summary>
    [[nodiscard]] size_t findFirstWhitespace(std::string_view str) noexcept;

    /// <summary>
    /// Returns the index of the first non-whitespace character in the string view.
    /// Returns std::string_view::npos if there are no non-whitespace characters.
    /// </summary>
    [[nodiscard]] size_t findFirstNonWhitespace(std::string_view str) noexcept;
}

#endif