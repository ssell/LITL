#ifndef LITL_CORE_STRING_ID_H__
#define LITL_CORE_STRING_ID_H__

#include "litl-core/hash.hpp"

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace litl
{
    /// <summary>
    /// Provides a hashed id for a string.
    /// </summary>
    struct StringId
    {
        constexpr StringId() = default;
        explicit constexpr StringId(uint64_t v) noexcept : value(v) {}
        explicit constexpr StringId(std::string_view str) noexcept : value(fastHashString(str)) {}

        friend constexpr bool operator==(StringId, StringId) noexcept = default;
        friend constexpr auto operator<=>(StringId, StringId) noexcept = default;

        uint64_t value = 0;
    };

    /// <summary>
    /// Compile time evaluator of a string to a StringId.
    /// Usage example: "Hello, World!"_sid
    /// </summary>
    consteval StringId operator""_sid(char const* str, std::size_t n)
    {
        return StringId{ std::string_view{str, n} };
    }

    /// <summary>
    /// StringId hasher for use with std::unordered_map
    /// </summary>
    struct StringIdHash
    {
        /// <summary>
        /// Needed for unordered-container heterogeneous-lookup path.
        /// </summary>
        using is_transparent = void;
        
        constexpr uint64_t operator()(StringId id) const noexcept
        {
            return id.value;
        }

        constexpr uint64_t operator()(std::string_view str) const noexcept
        {
            return fastHashString(str);
        }
    };

    /// <summary>
    /// StringId equality for use with std::unordered_map
    /// </summary>
    struct StringIdEquals
    {
        /// <summary>
        /// Needed for unordered-container heterogeneous-lookup path.
        /// </summary>
        using is_transparent = void;

        constexpr bool operator()(StringId a, StringId b) const noexcept
        {
            return a.value == b.value;
        }

        constexpr bool operator()(StringId a, std::string_view b) const noexcept
        {
            return a.value == fastHashString(b);
        }

        constexpr bool operator()(std::string_view a, StringId b) const noexcept
        {
            return fastHashString(a) == b.value;
        }
    };

    template<class V>
    using StringIdMap = std::unordered_map<StringId, V, StringIdHash, StringIdEquals>;
}

#endif