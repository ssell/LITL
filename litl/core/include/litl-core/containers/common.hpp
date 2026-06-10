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
}

#endif