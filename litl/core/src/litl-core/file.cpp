#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    File::File()
    {
        // ... placeholder ...
    }

    File::File(std::string_view path)
    {
        m_file = path.data();
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