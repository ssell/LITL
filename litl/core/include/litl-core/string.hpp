#ifndef LITL_CORE_STRING_H__
#define LITL_CORE_STRING_H__

#include <cctype>
#include <string>
#include <string_view>

namespace litl
{
    inline std::string toLowercase(std::string_view str) noexcept
    {
        std::string lowered(str);

        for (auto& c : lowered)
        {
            c = std::tolower(static_cast<unsigned char>(c));
        }

        return lowered;
    }
}

#endif