#ifndef LITL_MATH_COLOR_H__
#define LITL_MATH_COLOR_H__

#include "litl-core/math/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "litl-core/types.hpp"

namespace litl
{
    struct color
    {
        constexpr color() noexcept {}
        constexpr color(color const& other) noexcept : value(other.value) {}
        constexpr explicit color(glm::vec4 const& other) noexcept : value(other) {}
        constexpr color(float r, float g, float b) noexcept : value{ r, g, b, 1.0f } {}
        constexpr color(float r, float g, float b, float a) noexcept : value{ r, g, b, a } {}

        // ---------------------------------------------------------------------------------
        // Accessors
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float& r() noexcept { return value.x; }
        [[nodiscard]] constexpr float const& r() const noexcept { return value.x; }
        [[nodiscard]] constexpr float& g() noexcept { return value.y; }
        [[nodiscard]] constexpr float const& g() const noexcept { return value.y; }
        [[nodiscard]] constexpr float& b() noexcept { return value.z; }
        [[nodiscard]] constexpr float const& b() const noexcept { return value.z; }
        [[nodiscard]] constexpr float& a() noexcept { return value.w; }
        [[nodiscard]] constexpr float const& a() const noexcept { return value.w; }

        [[nodiscard]] constexpr glm::vec4& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::vec4 const& data() const noexcept
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

        glm::vec4 value{ 0.0f, 0.0f, 0.0f, 1.0f };
    };

    static_assert(std::is_nothrow_copy_constructible_v<color>);
    static_assert(std::is_nothrow_move_constructible_v<color>);
    static_assert(std::is_nothrow_copy_assignable_v<color>);
    static_assert(std::is_nothrow_move_assignable_v<color>);

    namespace colors
    {
        static constexpr color Black  { 0.0f, 0.0f, 0.0f, 1.0f };
        static constexpr color White  { 1.0f, 1.0f, 1.0f, 1.0f };
        static constexpr color Clear  { 0.0f, 0.0f, 0.0f, 0.0f };
        static constexpr color Red    { 1.0f, 0.0f, 0.0f, 1.0f };
        static constexpr color Green  { 0.0f, 1.0f, 0.0f, 1.0f };
        static constexpr color Blue   { 0.0f, 0.0f, 1.0f, 1.0f };
        static constexpr color Yellow { 1.0f, 1.0f, 0.0f, 1.0f };
        static constexpr color Pink   { 1.0f, 0.0f, 1.0f, 1.0f };
    }
}

LITL_REGISTER_TYPE_NAME(litl::color)

#endif