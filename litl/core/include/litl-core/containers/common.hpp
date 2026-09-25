#ifndef LITL_CORE_CONTAINERS_COMMON_H__
#define LITL_CORE_CONTAINERS_COMMON_H__

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <optional>
#include <ranges>
#include <span>
#include <type_traits>
#include <vector>

#include "litl-core/assert.hpp"

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
    template<typename T> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] std::span<std::byte const, sizeof(T)> as_byte_span(T const& obj) noexcept
    {
        return std::as_bytes(std::span<T const, 1>{std::addressof(obj), 1});
    }

    /// <summary>
    /// Converts a generic void pointer with a given size to a const byte span.
    /// </summary>
    [[nodiscard]] static std::span<std::byte const> generic_as_byte_span(void const* data, size_t size)
    {
        return std::span<std::byte const>{ reinterpret_cast<const std::byte*>(data), size };
    }

    template<typename T> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] std::optional<T> from_generic_byte_span(std::span<std::byte const> bytes, size_t offset) noexcept
    {
        if ((offset + sizeof(T)) > bytes.size_bytes())
        {
            return std::nullopt;
        }

        T obj;
        std::memcpy(&obj, bytes.data() + offset, sizeof(T));

        return obj;
    }

    /// <summary>
    /// Given a target vector, removes a list of sorted and unique indices from it.
    /// This is an O(n) implementation that does at most one move per surviving element, and never moves an element more than once.
    /// </summary>
    template<typename T>
    void eraseVectorIndices(std::vector<T>& targetVector, std::vector<std::size_t> const& sortedUniqueIndices) noexcept
    {
        if (sortedUniqueIndices.empty())
        {
            return;
        }
        
        size_t i = 0ull;
        size_t indexPos = 0ull;

        std::erase_if(targetVector, [&](const T&) 
        {
            i++;

            if ((indexPos < sortedUniqueIndices.size()) && ((i - 1) == sortedUniqueIndices[indexPos]))
            {
                ++indexPos;
                return true;
            }

            return false;
        });
    }
}

#endif