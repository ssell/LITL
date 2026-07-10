#ifndef LITL_CORE_CONTAINERS_ALIGNED_BYTE_BUFFER_H__
#define LITL_CORE_CONTAINERS_ALIGNED_BYTE_BUFFER_H__

#include <cstdint>
#include <span>

namespace litl
{
    template<std::size_t Alignment>
    class AlignedByteBuffer final
    {
        static_assert(Alignment > 0 && (Alignment & (Alignment - 1)) == 0, "Alignment must be a power of two");

    public:

        explicit AlignedByteBuffer(std::size_t sizeBytes)
            : m_size(sizeBytes)
        {
            m_data = static_cast<std::byte*>(::operator new(sizeBytes, std::align_val_t{ Alignment }));
        }

        AlignedByteBuffer(AlignedByteBuffer const&) = delete;

        AlignedByteBuffer(AlignedByteBuffer&& other) noexcept
        {

        }

        ~AlignedByteBuffer()
        {
            if (m_data != nullptr)
            {
                ::operator delete(m_data, std::align_val_t{ Alignment });
            }
        }

        [[nodiscard]] std::span<std::byte> bytes() noexcept
        {
            return { m_data, m_size };
        }

        [[nodiscard]] std::span<std::byte const> bytes() const noexcept
        {
            return { m_data, m_size };
        }

        [[nodiscard]] size_t size() const noexcept
        {
            return m_size;
        }

        template<typename T> requires (alignof(T) <= Alignment)
        [[nodiscard]] std::span<T> as() noexcept
        {
            return { reinterpret_cast<T*>(m_data), m_size / sizeof(T) };
        }

        template<typename T> requires (alignof(T) <= Alignment)
        [[nodiscard]] std::span<T const> as() const noexcept
        {
            return { reinterpret_cast<T const*>(m_data), m_size / sizeof(T) };
        }

    private:

        std::byte* m_data = nullptr;
        std::size_t m_size = 0;
    };
}

#endif