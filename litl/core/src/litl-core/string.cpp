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

    bool isWhitespace(char c) noexcept
    {
        return g_whitespace.find(c) != std::string_view::npos;
    }

    size_t findFirstWhitespace(std::string_view str) noexcept
    {
        return str.find_first_of(g_whitespace);
    }

    size_t findFirstNonWhitespace(std::string_view str) noexcept
    {
        return str.find_first_not_of(g_whitespace);
    }

    bool stringsEquals(std::string_view a, std::string_view b, bool ignoreCase) noexcept
    {
        const size_t size = a.size();

        if (size != b.size())
        {
            return false;
        }

        const char* __restrict pa = a.data();
        const char* __restrict pb = b.data();

        if (ignoreCase)
        {
            for (size_t i = 0ull; i < size; ++i)
            {
                const auto achar = static_cast<unsigned char>(pa[i]);
                const auto bchar = static_cast<unsigned char>(pb[i]);

                if (std::tolower(achar) != std::tolower(bchar))
                {
                    return false;
                }
            }
        }
        else
        {
            for (size_t i = 0ull; i < size; ++i)
            {
                if (pa[i] != pb[i])
                {
                    return false;
                }
            }
        }

        return true;
    }

    bool stringsEqualFirstLowercase(std::string_view lowercase, std::string_view unknowncase) noexcept
    {
        const size_t size = lowercase.size();

        if (size != unknowncase.size())
        {
            return false;
        }

        const char* __restrict plower = lowercase.data();          // Use __restrict to tell the compiler that no other pointer will overlap with the memory region.
        const char* __restrict punknown = unknowncase.data();      // This allows it to cache the values in registers, reorder instructions, and auto-vectorize any loops.

        for (size_t i = 0ull; i < size; ++i)
        {
            const auto lchar = static_cast<unsigned char>(plower[i]);
            const auto uchar = static_cast<unsigned char>(punknown[i]);

            if (std::tolower(uchar) != lchar)
            {
                return false;
            }
        }

        return true;
    }
}