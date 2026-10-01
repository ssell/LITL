#include "litl-core/string.hpp"

namespace litl
{
    namespace
    {
        inline constexpr std::string_view g_whitespace{ " \t\v\f\r" };
    }

    std::string toLowercase(std::string_view str) noexcept
    {
        std::string lowered(str);

        for (auto& c : lowered)
        {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        return lowered;
    }

    std::string_view trimLeadingWhitespace(std::string_view str) noexcept
    {
        auto const first = str.find_first_not_of(g_whitespace);
        return (first == std::string_view::npos) ? std::string_view{} : str.substr(first);
    }
}