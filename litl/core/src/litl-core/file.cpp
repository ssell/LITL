#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
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

    File::File(std::string_view path)
        : m_file(path)
    {
        refresh();
    }

    File::File(std::filesystem::directory_entry const& entry) : 
        m_file(entry.path()), 
        m_lastWriteTime(to_time_t(entry.last_write_time())),
        m_fileBytes(static_cast<uint32_t>(entry.file_size()))
    {

    }

    void File::refresh() noexcept
    {
        m_lastWriteTime = to_time_t(std::filesystem::last_write_time(m_file));
        m_fileBytes = std::filesystem::file_size(m_file);
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
        return m_lastWriteTime;
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