#include <algorithm>
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

    bool Directory::deleteRecursive(std::string_view path) noexcept
    {
        std::error_code error;

        if (std::filesystem::remove_all(path, error))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    std::string Directory::absolutePath(std::string_view directory) noexcept
    {
        std::filesystem::path directoryPath = directory;
        return std::filesystem::weakly_canonical(directoryPath).string();
    }

    std::string Directory::appendFolder(std::string_view directory, std::string_view folder) noexcept
    {
        std::filesystem::path directoryPath = directory;
        directoryPath /= folder;
        return std::filesystem::weakly_canonical(directoryPath).string();
    }

    uint32_t Directory::fileCount(std::string_view directory, bool recursive) noexcept
    {
        if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) 
        {
            return 0u;
        }

        if (recursive)
        {
            auto start = std::filesystem::recursive_directory_iterator(directory);
            auto end = std::filesystem::recursive_directory_iterator();

            return std::count_if(start, end, [](const auto& entry) {
                return entry.is_regular_file();         // Excludes directories, symlinks, etc.
            });
        }
        else
        {
            auto start = std::filesystem::directory_iterator(directory);
            auto end = std::filesystem::directory_iterator();

            return std::count_if(start, end, [](const auto& entry) {
                return entry.is_regular_file();
            });
        }
    }
}