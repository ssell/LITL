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
        [[nodiscard]] std::string relativeTo(std::string_view parentDir) const noexcept;

        /// <summary>
        /// Returns if the file exists or not.
        /// If there was an error checking the file then std::nullopt will be returned instead.
        /// </summary>
        [[nodiscard]] bool exists() const noexcept;

        [[nodiscard]] static bool exists(std::string_view path) noexcept;

        /// <summary>
        /// Reads the contents of the file synchronously.
        /// If there was an error reading the file then std::nullopt will be returned instead.
        /// </summary>
        [[nodiscard]] std::optional<std::vector<std::byte>> readAllBytes() const noexcept;

        /// <summary>
        /// 
        /// </summary>
        bool writeAllBytes(std::span<std::byte const> bytes) const noexcept;

        /// <summary>
        /// Reads the contents of the file synchronously.
        /// Returns false if there was an error reading the file.
        /// </summary>
        [[nodiscard]] bool readAllBytes(std::vector<std::byte>& bytes) const noexcept;

        /// <summary>
        /// Returns true if the provided filename is on the internal reserved list. This is the name excluding extension or path.
        /// This check is case insensitive. For example both "con" and "CON" would return true.
        /// </summary>
        [[nodiscard]] static bool IsReservedFileName(std::string_view name) noexcept;

        /// <summary>
        /// Strips/replaces all characters unsafe for a file name. This includes: slashes, colon, whitespace runs, trailing dots and spaces, and operating system specific sets such as *?"<>|
        /// </summary>
        [[nodiscard]] static std::string SanitizeFilename(std::string_view name) noexcept;

    private:

        std::filesystem::path m_file;
        std::filesystem::file_time_type m_lastWriteTime{};
        uint32_t m_fileBytes{ 0u };
    };
}

#endif