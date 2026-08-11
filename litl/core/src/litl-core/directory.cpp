#include <filesystem>
#include "litl-core/directory.hpp"

namespace litl
{
    bool Directory::exists(std::string_view path) noexcept
    {
        std::error_code error;

        if (std::filesystem::is_directory(path, error))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool Directory::ensureExists(std::string_view path) noexcept
    {
        std::error_code error;

        if (std::filesystem::create_directories(path, error))
        {
            return true;
        }
        else
        {
            return exists(path);
        }
    }
}