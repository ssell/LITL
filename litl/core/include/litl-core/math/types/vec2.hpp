#ifndef LITL_MATH_VEC2_H__
#define LITL_MATH_VEC2_H__

#include <cassert>
#include <format>
#include <string>

#include "litl-core/types.hpp"
#include "litl-core/math/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "litl-core/math/common.hpp"

namespace litl
{
    struct vec2
    {
        constexpr vec2() noexcept {}
        constexpr vec2(vec2 const& other) noexcept : value(other.value) {}
        constexpr explicit vec2(float xy) noexcept : value{ xy, xy } {}
        constexpr explicit vec2(glm::vec2 const& other) noexcept : value(other) {}
        constexpr vec2(float x, float y) noexcept : value{ x, y } {}

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(vec2 const& other) const noexcept
        {
            return fequals(value.x, other.value.x) && fequals(value.y, other.value.y);
        }

        [[nodiscard]] constexpr bool operator==(glm::vec2 const& other) const noexcept
        {
            return fequals(value.x, other.x) && fequals(value.y, other.y);
        }

        // ---------------------------------------------------------------------------------
        // Accessors
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float& x() noexcept
        {
            return value.x;
        }

        [[nodiscard]] constexpr float const& x() const noexcept
        {
            return value.x;
        }

        [[nodiscard]] constexpr float& y() noexcept
        {
            return value.y;
        }

        [[nodiscard]] constexpr float const& y() const noexcept
        {
            return value.y;
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        std::string toString() const noexcept
        {
            return std::format("({:.3f},{:.3f})", value.x, value.y);
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::vec2& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::vec2 const& data() const noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr float* dataPtr() noexcept
        {
            return glm::value_ptr(value);
        }

        [[nodiscard]] constexpr float const* dataPtr() const noexcept
        {
            return glm::value_ptr(value);
        }

    private:

        glm::vec2 value{ 0.0f, 0.0f };
    };

    static_assert(std::is_nothrow_copy_constructible_v<vec2>);
    static_assert(std::is_nothrow_move_constructible_v<vec2>);
    static_assert(std::is_nothrow_copy_assignable_v<vec2>);
    static_assert(std::is_nothrow_move_assignable_v<vec2>);
}

LITL_REGISTER_TYPE_NAME(litl::vec2)

#endif