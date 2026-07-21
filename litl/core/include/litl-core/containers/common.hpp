#ifndef LITL_CORE_CONTAINERS_COMMON_H__
#define LITL_CORE_CONTAINERS_COMMON_H__

#include <span>
#include <ranges>
#include <cstddef>
#include <type_traits>

namespace litl
{
    /// <summary>
    /// Converts a range (array, vector, etc.) to a const byte span.
    /// </summary>
    template<std::ranges::contiguous_range R>
    [[nodiscard]] auto as_byte_span(R&& r) -> std::span<std::byte const>
    {
        return std::as_bytes(std::span{ std::forward<R>(r) });
    }

    /// <summary>
    /// Converts a range (array, vector, etc.) to a writeable byte span.
    /// </summary>
    template<std::ranges::contiguous_range R>
    [[nodiscard]] auto as_writable_byte_span(R&& r) -> std::span<std::byte>
    {
        return std::as_writable_bytes(std::span{ std::forward<R>(r) });
    }

    /// <summary>
    /// Converts a POD to a const byte span.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="obj"></param>
    /// <returns></returns>
    template<typename T> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] std::span<std::byte const, sizeof(T)> as_byte_span(T const& obj) noexcept
    {
        return std::as_bytes(std::span<T const, 1>{std::addressof(obj), 1});
    }

    /// <summary>
    /// Converts a generic void pointer with a given size to a const byte span.
    /// </summary>
    /// <param name="data"></param>
    /// <param name="size"></param>
    /// <returns></returns>
    [[nodiscard]] static std::span<std::byte const> generic_as_byte_span(void* data, size_t size)
    {
        return std::span<std::byte const>{ reinterpret_cast<const std::byte*>(data), size };
    }
}

#endif