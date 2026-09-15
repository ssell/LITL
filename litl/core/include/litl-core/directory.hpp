#ifndef LITL_CORE_DIRECTORY_H__
#define LITL_CORE_DIRECTORY_H__

#include <string>
#include <string_view>

namespace litl
{
    class Directory
    {
    public:

        [[nodiscard]] static bool exists(std::string_view path) noexcept;
        [[nodiscard]] static bool ensureExists(std::string_view path) noexcept;
        [[nodiscard]] static bool deleteRecursive(std::string_view path) noexcept;
        [[nodiscard]] static std::string absolutePath(std::string_view directory) noexcept;
        [[nodiscard]] static std::string appendFolder(std::string_view directory, std::string_view folder) noexcept;
    };
}

#endif