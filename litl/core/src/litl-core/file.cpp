#include <cctype>
#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "litl-core/file.hpp"
#include "litl-core/string.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/containers/flatHashSet.hpp"

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

    bool File::erase() noexcept
    {
        if (exists())
        {
            std::error_code error{};

            if (std::filesystem::remove(m_file, error))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        return false;
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


    std::string File::parentFolderPath() const noexcept
    {
        return std::filesystem::canonical(m_file.parent_path()).string();
    }

    std::string File::name() const noexcept
    {
        return m_file.stem().string();
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

    bool File::exists() const noexcept
    {
        std::error_code error;
        bool exists = std::filesystem::exists(m_file, error);

        if (error)
        {
            logWarning("Attempt to check if the file at '", m_file.string(), "' exists failed with error code ", error.value());
        }

        return exists;
    }

    bool File::exists(std::string_view path) noexcept
    {
        File file(path);
        return file.exists();
    }

    std::optional<std::vector<std::byte>> File::readAllBytes() const noexcept
    {
        std::vector<std::byte> bytes;

        if (readAllBytes(bytes))
        {
            return bytes;
        }

        return std::nullopt;
    }

    bool File::writeAllBytes(std::span<std::byte const> bytes) const noexcept
    {
        std::ofstream outStream(m_file.string(), std::ios::out | std::ios::binary);

        if (!outStream)
        {
            return false;
        }

        outStream.write(reinterpret_cast<char const*>(bytes.data()), bytes.size());
        outStream.close();

        return true;
    }

    bool File::readAllBytes(std::vector<std::byte>& bytes) const noexcept
    {
        if (!exists())
        {
            logWarning("Attempt to read bytes for file at '", m_file.string(), "' failed as it does not exist.");
            return false;
        }

        std::ifstream instream(absolutePath(), std::ios::ate | std::ios::binary);

        if (!instream.is_open())
        {
            logWarning("Failed to open file at '", m_file.string(), "' to read bytes.");
            return false;
        }

        const auto size = static_cast<size_t>(instream.tellg());
        instream.seekg(0);
        bytes.resize(size);

        if (!instream.read(reinterpret_cast<char*>(bytes.data()), size))
        {
            return false;
        }

        return true;
    }

    namespace
    {
        static const FlatHashSet<uint64_t> g_ReservedFileNames{
            // Windows reserved file names
            "con"_sid.value,
            "nul"_sid.value,
            "aux"_sid.value,
            "prn"_sid.value,
            "com1"_sid.value,
            "com2"_sid.value,
            "com3"_sid.value,
            "com4"_sid.value,
            "com5"_sid.value,
            "com6"_sid.value,
            "com7"_sid.value,
            "com8"_sid.value,
            "com9"_sid.value,
            "lpt1"_sid.value,
            "lpt2"_sid.value,
            "lpt3"_sid.value,
            "lpt4"_sid.value,
            "lpt5"_sid.value,
            "lpt6"_sid.value,
            "lpt7"_sid.value,
            "lpt8"_sid.value,
            "lpt9"_sid.value
        };
    }

    bool File::IsReservedFileName(std::string_view name) noexcept
    {
        return g_ReservedFileNames.contains(StringId(toLowercase(name)).value);
    }

    std::string File::SanitizeFilename(std::string_view name) noexcept
    {
        static constexpr std::string_view forbidden = R"(/\:*?"<>|)";

        if (name.empty())
        {
            return "";
        }

        std::string sanitized;
        sanitized.reserve(name.size());
        bool pendingSpace = false;

        for (unsigned char c : name)
        {
            if (std::isspace(c))
            {
                pendingSpace = true;            // We will collapse whitespace runs into a single '-'
            }
            else
            {
                if (pendingSpace)
                {
                    if (!sanitized.empty())     // Dont start with a '-'
                    {
                        sanitized += '-';
                    }

                    pendingSpace = false;
                }

                if (forbidden.contains(static_cast<char>(c)) || (c < 0x20))
                {
                    sanitized += '_';           // Structurally illegal character -> underscore
                }
                else
                {
                    sanitized += static_cast<char>(c);
                }
            }
        }

        // Windows disallows trailing '.' and ' ', and we strip any trailing '-' as well in case trailing whitespace was collapsed.
        while (!sanitized.empty() && (sanitized.back() == '.' || sanitized.back() == ' ' || sanitized.back() == '-'))
        {
            sanitized.pop_back();
        }

        return sanitized;
    }
}
