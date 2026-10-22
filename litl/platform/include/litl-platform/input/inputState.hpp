#ifndef LITL_PLATFORM_INPUT_STATE_H__
#define LITL_PLATFORM_INPUT_STATE_H__

#include <bitset>
#include <cstdint>

#include "litl-core/math/types/vec2.hpp"
#include "litl-platform/input/inputEnums.hpp"

namespace litl
{
    struct KeyboardState
    {
        /// <summary>
        /// Was the key pressed this frame? This is not exclusive to released and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> pressed{};

        /// <summary>
        /// The state of the button at the last poll and is directly set by whether the last received event for the key was a press or release.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> down{};

        /// <summary>
        /// Was the key released this frame? This is not exclusive to pressed and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> released{};

        /// <summary>
        /// Clears the pressed and released states, but keeps the down states.
        /// A key being held down can be interpreted as one that is down but was not pressed this frame.
        /// </summary>
        void reset() noexcept
        {
            pressed.reset();
            released.reset();
        }
    };

    struct MousePosition
    {
        /// <summary>
        /// The current position of the mouse.
        /// </summary>
        vec2 xy{};

        /// <summary>
        /// The change in mouse position since the last capture.
        /// </summary>
        vec2 xyDelta{};

        /// <summary>
        /// Did the mouse enter the window since the last capture?
        /// </summary>
        bool enteredWindow{ false };

        /// <summary>
        /// Is the mouse currently in the window?
        /// </summary>
        bool inWindow{ false };

        /// <summary>
        /// Did the mouse exit the window since the last capture?
        /// </summary>
        bool exitedWindow{ false };

        void reset()
        {
            enteredWindow = false;
            exitedWindow = false;
            xyDelta.setZero();
        }
    };

    struct MouseState
    {
        /// <summary>
        /// Was the button pressed this frame? This is not exclusive to released and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> pressed{};

        /// <summary>
        /// The state of the button at the last poll and is directly set by whether the last received event for the key was a press or release.
        /// </summary>
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> down{};

        /// <summary>
        /// Was the button released this frame? This is not exclusive to pressed and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(MouseButton::Count)> released{};

        /// <summary>
        /// Position information for the mouse.
        /// </summary>
        MousePosition position{};

        /// <summary>
        /// Change in xy offset of the scroll wheel since the last capture.
        /// .x = horizontal scroll amount, which is typically 0.
        /// .y = vertical scroll amout, where positive is up/forward and negative is down/backward.
        /// </summary>
        vec2 scroll{};

        /// <summary>
        /// Current mouse capture mode.
        /// </summary>
        CursorMode mode{ CursorMode::Normal };

        /// <summary>
        /// Clears the pressed and released states, but keeps the down states.
        /// A button being held down can be interpreted as one that is down but was not pressed this frame.
        /// </summary>
        void reset() noexcept
        {
            pressed.reset();
            released.reset();
            position.reset();
            scroll.setZero();
        }
    };

    struct GamepadState
    {
        /// <summary>
        /// Was the button pressed this frame? This is not exclusive to released and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> pressed{};

        /// <summary>
        /// The state of the button at the last poll and is directly set by whether the last received event for the key was a press or release.
        /// </summary>
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> down{};

        /// <summary>
        /// Was the button released this frame? This is not exclusive to pressed and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(GamepadButton::Count)> released{};

        /// <summary>
        /// Clears the pressed and released states, but keeps the down states.
        /// A button being held down can be interpreted as one that is down but was not pressed this frame.
        /// </summary>
        void reset() noexcept
        {
            pressed.reset();
            released.reset();
        }
    };

    /// <summary>
    /// The current state of input for a frame.
    /// This is the raw state the simply records which buttons on currently down and does not discern between press, held, or release.
    /// </summary>
    struct InputState
    {
        KeyboardState keyboard{};
        MouseState mouse{};
        GamepadState gamepad{};

        void reset() noexcept
        {
            keyboard.reset();
            mouse.reset();
            gamepad.reset();
        }
    };
}

#endif