#ifndef LITL_CORE_FILE_H__
#define LITL_CORE_FILE_H__

#include <ctime>
#include <filesystem>

namespace litl
{
    class File
    {
    public:

        File();
        File(std::string_view path);
        File(std::filesystem::directory_entry const& entry);

        /// <summary>
        /// Refereshes tracked information about the file such as size and last write time.
        /// </summary>
        void refresh() noexcept;

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
        /// Returns the file extension. This includes the "."
        /// For example "image.png" would return ".png" and "file.tar.gz" would return ".gz"
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::string extension() const noexcept;

        /// <summary>
        /// Size of the file in bytes.
        /// This value can be refreshed if out-of-date using refresh().
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t size() const noexcept;

        /// <summary>
        /// Last time the file was written to.
        /// This value can be refreshed if out-of-date using refresh().
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::time_t lastWriteTime() const noexcept;

        /// <summary>
        /// Returns the path relative to a parent directory.
        /// For example a file "C:\\Projects\\LITL\\assets\\mesh\\triangle.fbx" relative to "assets" would return "mesh\\triangle.fbx".
        /// </summary>
        /// <param name="parentDir"></param>
        /// <returns></returns>
        [[nodiscard]] std::string relativeTo(std::string_view parentDir) const noexcept;

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
        std::time_t m_lastWriteTime{};
        uint32_t m_fileBytes{ 0u };
    };
}

#endif