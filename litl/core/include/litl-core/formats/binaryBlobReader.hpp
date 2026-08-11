#ifndef LITL_CORE_FORMATS_BINARY_BLOB_READER_H__
#define LITL_CORE_FORMATS_BINARY_BLOB_READER_H__

#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>

namespace litl
{
    /// <summary>
    /// Utility for traversing a blob of binary data that is composed of a single fixed type.
    /// </summary>
    class BinaryBlobReader final
    {
    public:

        explicit BinaryBlobReader(std::span<std::byte const> blob) noexcept : m_blob(blob) {}

        template<typename T> requires std::is_trivially_copyable_v<T>
        [[nodiscard]] bool read(T& out) noexcept
        {
            if (sizeof(T) > (m_blob.size() - m_cursor))
            {
                // Reached the end.
                return false;
            }

            std::memcpy(&out, m_blob.data() + m_cursor, sizeof(T));
            m_cursor += sizeof(T);

            return true;
        }
        
    private:

        std::span<std::byte const> m_blob;
        size_t m_cursor{ 0ull };
    };
}

#endif