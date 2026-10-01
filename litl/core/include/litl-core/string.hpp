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
    /// <param name="str"></param>
    /// <returns></returns>
    [[nodiscard]] std::string toLowercase(std::string_view str) noexcept;

    /// <summary>
    /// Returns a view of the string with the leading whitespace ommitted.
    /// </summary>
    /// <param name="str"></param>
    /// <returns></returns>
    [[nodiscard]] std::string_view trimLeadingWhitespace(std::string_view str) noexcept;
}

#endif