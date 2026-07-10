#ifndef LITL_CORE_ENUM_BIT_FLAGS_H__
#define LITL_CORE_ENUM_BIT_FLAGS_H__

#include <type_traits>
#include <concepts>

namespace litl
{
    template <typename E>
    struct enable_bitmask : std::false_type {};

    template <typename E>
    concept bitmask_enum = std::is_enum_v<E> && enable_bitmask<E>::value;

    // Forbid signed underlying types: ~ and shifts on them invite UB.
    template <bitmask_enum E>
    constexpr auto to_u(E e) noexcept
    {
        static_assert(std::is_unsigned_v<std::underlying_type_t<E>>,
            "bitmask enums must have an unsigned underlying type");
        return static_cast<std::underlying_type_t<E>>(e);
    }

    template <bitmask_enum E>
    [[nodiscard]] constexpr bool any(E e) noexcept { return to_u(e) != 0; }
    template <bitmask_enum E>
    [[nodiscard]] constexpr bool none(E e) noexcept { return to_u(e) == 0; }
    template <bitmask_enum E>
    [[nodiscard]] constexpr bool has_all(E e, E mask) noexcept { return (to_u(e) & to_u(mask)) == to_u(mask); }
    template <bitmask_enum E>
    [[nodiscard]] constexpr bool has_any(E e, E mask) noexcept { return (to_u(e) & to_u(mask)) != 0; }
}

// Global scope so unqualified lookup finds them regardless of the enum's namespace.
template <litl::bitmask_enum E>
[[nodiscard]] constexpr E operator|(E a, E b) noexcept { return static_cast<E>(litl::to_u(a) | litl::to_u(b)); }
template <litl::bitmask_enum E>
[[nodiscard]] constexpr E operator&(E a, E b) noexcept { return static_cast<E>(litl::to_u(a) & litl::to_u(b)); }
template <litl::bitmask_enum E>
[[nodiscard]] constexpr E operator^(E a, E b) noexcept { return static_cast<E>(litl::to_u(a) ^ litl::to_u(b)); }
template <litl::bitmask_enum E>
[[nodiscard]] constexpr E operator~(E a)      noexcept { return static_cast<E>(~litl::to_u(a)); }

template <litl::bitmask_enum E>
constexpr E& operator|=(E& a, E b) noexcept { return a = a | b; }
template <litl::bitmask_enum E>
constexpr E& operator&=(E& a, E b) noexcept { return a = a & b; }
template <litl::bitmask_enum E>
constexpr E& operator^=(E& a, E b) noexcept { return a = a ^ b; }

#define LITL_ENABLE_BITMASK(E) \
    template <> struct enable_bitmask<E> : std::true_type {}

#endif