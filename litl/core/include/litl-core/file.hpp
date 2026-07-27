#ifndef LITL_CORE_FILE_H__
#define LITL_CORE_FILE_H__

#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace litl
{
    class File
    {
    public:

        File(std::span<char const> path);

        /// <summary>
        /// Returns the path to the file local from the current directory that the application is running from.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::string localPath() const noexcept;

        /// <summary>
        /// Returns the absolute path to the file.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::string absolutePath() const noexcept;

        /// <summary>
        /// Returns if the file exists or not.
        /// If there was an error checking the file then std::nullopt will be returned instead.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::optional<bool> exists() const noexcept;

        /// <summary>
        /// Reads the contents of the file synchronously.
        /// If there was an error reading the file then std::nullopt will be returned instead.
        /// </summary>
        /// <param name="alignment"></param>
        /// <returns></returns>
        [[nodiscard]] std::optional<std::vector<std::byte>> readAllBytes() const noexcept;

    private:

        std::filesystem::path m_file;
    };
}

#endif