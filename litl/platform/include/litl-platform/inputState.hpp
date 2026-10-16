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
        std::bitset<static_cast<uint32_t>(Key::Count)> keysDown;
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> gamepadButtonsDown;
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> mouseButtonsDown;
        vec2 cursorPosition{};
    };
}

#endif