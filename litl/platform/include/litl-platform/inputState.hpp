#ifndef LITL_PLATFORM_INPUT_STATE_H__
#define LITL_PLATFORM_INPUT_STATE_H__

#include <bitset>
#include <cstdint>

#include "litl-core/math/types/vec2.hpp"
#include "litl-platform/inputEnums.hpp"

namespace litl
{
    /// <summary>
    /// The current state of input for a frame.
    /// This is the raw state the simply records which buttons on currently down and does not discern between press, held, or release.
    /// </summary>
    struct InputState
    {
        vec2 cursorPosition{};

        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> gamepadButtonsDown;
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> gamepadButtonsPressed;
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> gamepadButtonsReleased;

        std::bitset<static_cast<uint32_t>(Key::Count)> keysDown;
        std::bitset<static_cast<uint32_t>(Key::Count)> keysPressed;
        std::bitset<static_cast<uint32_t>(Key::Count)> keysReleased;

        std::bitset<static_cast<uint32_t>(MouseButton::Count)> mouseButtonsDown;
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> mouseButtonsPressed;
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> mouseButtonsReleased;

        void reset() noexcept
        {
            keysDown.reset();
            gamepadButtonsDown.reset();
            mouseButtonsDown.reset();
        }
    };
}

#endif