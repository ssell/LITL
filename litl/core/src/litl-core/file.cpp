#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    namespace
    {
        std::time_t to_time_t(std::filesystem::file_time_type const& fileTime) noexcept
        {
            // love the brevity of C++
            return std::chrono::system_clock::to_time_t(std::chrono::clock_cast<std::chrono::system_clock>(fileTime));
        }
    }

    File::File()
    {
        // ... placeholder ...
    }

    File::File(std::string_view path) : m_file(path)
    {
        refresh();
    }

    File::File(std::filesystem::directory_entry const& entry) : m_file(entry.path())
    {
        // Prefer refresh here instead of pulling straight from the directory_entry.
        // The directory_entry::file_size() and directory_entry::last_write_time() can throw exceptions.
        refresh();
    }

    bool File::refresh() noexcept
    {
        std::error_code error;

        m_lastWriteTime = std::filesystem::last_write_time(m_file, error);

        if (!error)
        {
            m_fileBytes = std::filesystem::file_size(m_file, error);
        }

        return (error.value() == 0);
    }

    std::string File::localPath() const noexcept
    {
        return m_file.relative_path().string();
    }

    std::string File::absolutePath() const noexcept
    {
        return std::filesystem::canonical(m_file).string();
    }

    std::string File::extension() const noexcept
    {
        return m_file.extension().string();
    }

    uint32_t File::size() const noexcept
    {
        return m_fileBytes;
    }

    std::time_t File::lastWriteTime() const noexcept
    {
        // note we store std::filesystem::file_time_type instead of std::time_t as it is higher resolution
        return to_time_t(m_lastWriteTime);
    }

    bool File::wasUpdated() noexcept
    {
        auto lastWriteTime = m_lastWriteTime;

        if (refresh())
        {
            return (lastWriteTime < m_lastWriteTime);
        }
        else
        {
            // Return that the file was updated if we previously were not errored out. For example, if the file was deleted since the last time we checked.
            return (lastWriteTime > std::filesystem::file_time_type::min());
        }
    }

    std::string File::relativeTo(std::string_view parentDir) const noexcept
    {
        return m_file.lexically_relative(parentDir).string();
    }

    std::optional<bool> File::exists() const noexcept
    {
        std::error_code error;
        bool exists = std::filesystem::exists(m_file, error);

        if (!error)
        {
            return exists;
        }

        logWarning("Attempt to check if the file at '", m_file.string(), "' exists failed with error code ", error.value());

        return std::nullopt;
    }

    std::optional<std::vector<std::byte>> File::readAllBytes() const noexcept
    {
        if (!exists().has_value())
        {
            logWarning("Attempt read bytes for file at '", m_file.string(), "' failed as it does not exist.");
            return std::nullopt;
        }

        std::ifstream instream(absolutePath(), std::ios::ate | std::ios::binary);

        if (!instream.is_open())
        {
            logWarning("Failed to open file at '", m_file.string(), "' to read bytes.");
            return std::nullopt;
        }

        const auto size = static_cast<size_t>(instream.tellg());
        std::vector<std::byte> bytes(size);

        instream.seekg(0);
        
        if (!instream.read(reinterpret_cast<char*>(bytes.data()), size))
        {
            return std::nullopt;
        }

        return bytes;
    }
}