#ifndef LITL_CORE_DIRECTORY_H__
#define LITL_CORE_DIRECTORY_H__

#include <string_view>

namespace litl
{
    class Directory
    {
    public:

        [[nodiscard]] static bool exists(std::string_view path) noexcept;
        [[nodiscard]] static bool ensureExists(std::string_view path) noexcept;
    };
}

#endif