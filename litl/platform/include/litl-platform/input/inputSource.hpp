#ifndef LITL_INPUT_SOURCE_H__
#define LITL_INPUT_SOURCE_H__

#include <cstdint>

#include "litl-core/constants.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-platform/input/inputEnums.hpp"

namespace litl
{
    enum class InputSourceType : uint8_t
    {
        Unknown       = 0u,
        Key           = 1u,
        MouseButton   = 2u,
        MouseDelta    = 3u,
        MouseScroll   = 4u,
        GamepadButton = 5u,
        GamepadAxis   = 6u
    };

    struct InputSource
    {
        InputSourceType type{ InputSourceType::Unknown };
        uint32_t code{ Constants::uint32_null_index };

        bool operator==(InputSource const& other) const = default;

        [[nodiscard]] constexpr static InputSource fromKey(Key key) noexcept
        {
            return InputSource{
                .type = InputSourceType::Key,
                .code = static_cast<uint32_t>(key)
            };
        }

        [[nodiscard]] constexpr static InputSource fromMouseButton(MouseButton button) noexcept
        {
            return InputSource{
                .type = InputSourceType::MouseButton,
                .code = static_cast<uint32_t>(button)
            };
        }

        [[nodiscard]] constexpr static InputSource fromMouseDelta() noexcept
        {
            return InputSource{ .type = InputSourceType::MouseDelta };
        }

        [[nodiscard]] constexpr static InputSource fromMouseScroll() noexcept
        {
            return InputSource{ .type = InputSourceType::MouseScroll };
        }

        [[nodiscard]] constexpr static InputSource fromGamepadButton(GamepadButton button) noexcept
        {
            return InputSource{
                .type = InputSourceType::GamepadButton,
                .code = static_cast<uint32_t>(button)
            };
        }

        [[nodiscard]] constexpr static InputSource fromGamepadAxis(GamepadAxis axis) noexcept
        {
            return InputSource{
                .type = InputSourceType::GamepadAxis,
                .code = static_cast<uint32_t>(axis)
            };
        }
    };

    struct InputSourceState
    {
        bool pressed{ false };
        bool held{ false };
        bool released{ false };
        vec2 xyDelta{};

        [[nodiscard]] bool isActive() const noexcept
        {
            return (pressed || held || released || !xyDelta.isZeroed());
        }
    };
}

#endif