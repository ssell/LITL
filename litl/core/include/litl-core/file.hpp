#ifndef LITL_CORE_FILE_H__
#define LITL_CORE_FILE_H__

#include <ctime>
#include <filesystem>
#include <span>

namespace litl
{
    class File
    {
    public:

        File();
        File(std::string_view path);
        File(std::filesystem::directory_entry const& entry);

        /// <summary>
        /// Erases the file. Returns true if successfully erased, otherwise will return false.
        /// </summary>
        /// <returns></returns>
        bool erase() noexcept;

        /// <summary>
        /// Erases the file at the specified path if it exists. Returns true if successfully erased, otherwise will return false.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        static bool erase(std::string_view path) noexcept;

        /// <summary>
        /// Refereshes tracked information about the file such as size and last write time.
        /// </summary>
        /// <returns>Can return false if the refresh failed. For instance, if the file no longer exists.</returns>
        bool refresh() noexcept;

        /// <summary>
        /// Returns the path to the file local from the current directory that the application is running from.
        /// </summary>
        [[nodiscard]] std::string localPath() const noexcept;

        /// <summary>
        /// Returns the absolute path to the file.
        /// </summary>
        [[nodiscard]] std::string absolutePath() const noexcept;

        /// <summary>
        /// Returns the path of the parent folder for this file.
        /// </summary>
        [[nodiscard]] std::string parentFolderPath() const noexcept;

        /// <summary>
        /// Returns the name of the file, without the extension.
        /// </summary>
        [[nodiscard]] std::string name() const noexcept;

        /// <summary>
        /// Returns the file extension. This includes the "."
        /// For example "image.png" would return ".png" and "file.tar.gz" would return ".gz"
        /// </summary>
        [[nodiscard]] std::string extension() const noexcept;

        /// <summary>
        /// Size of the file in bytes.
        /// This value can be refreshed if out-of-date using refresh().
        /// </summary>
        [[nodiscard]] uint32_t size() const noexcept;

        /// <summary>
        /// Last time the file was written to.
        /// This value can be refreshed if out-of-date using refresh().
        /// </summary>
        [[nodiscard]] std::time_t lastWriteTime() const noexcept;

        /// <summary>
        /// Refreshes the write time of the file and returns true if it has been written to since the last write time.
        /// </summary>
        [[nodiscard]] bool wasUpdated() noexcept;

        /// <summary>
        /// Returns the path relative to a parent directory.
        /// For example a file "C:\\Projects\\LITL\\assets\\mesh\\triangle.fbx" relative to "assets" would return "mesh\\triangle.fbx".
        /// </summary>
        /// <param name="parentDir"></param>
        [[nodiscard]] std::string relativeTo(std::string_view parentDir) const noexcept;

        /// <summary>
        /// Returns if the file exists or not.
        /// If there was an error checking the file then std::nullopt will be returned instead.
        /// </summary>
        [[nodiscard]] std::optional<bool> exists() const noexcept;

        [[nodiscard]] static std::optional<bool> exists(std::string_view path) noexcept;

        /// <summary>
        /// Reads the contents of the file synchronously.
        /// If there was an error reading the file then std::nullopt will be returned instead.
        /// </summary>
        /// <param name="alignment"></param>
        [[nodiscard]] std::optional<std::vector<std::byte>> readAllBytes() const noexcept;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="bytes"></param>
        /// <returns></returns>
        bool writeAllBytes(std::span<std::byte const> bytes) const noexcept;

        /// <summary>
        /// Reads the contents of the file synchronously.
        /// Returns false if there was an error reading the file.
        /// </summary>
        /// <param name="bytes"></param>
        [[nodiscard]] bool readAllBytes(std::vector<std::byte>& bytes) const noexcept;

    private:

        std::filesystem::path m_file;
        std::filesystem::file_time_type m_lastWriteTime{};
        uint32_t m_fileBytes{ 0u };
    };
}

#endif