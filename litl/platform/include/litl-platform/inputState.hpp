#ifndef LITL_PLATFORM_INPUT_STATE_H__
#define LITL_PLATFORM_INPUT_STATE_H__

#include <bitset>
#include <cstdint>

#include "litl-core/math/types/vec2.hpp"
#include "litl-platform/inputEnums.hpp"

namespace litl
{
    struct KeyboardState
    {
        /// <summary>
        /// Was the key pressed this frame? This is not exclusive to released and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> pressed;

        /// <summary>
        /// The state of the button at the last poll and is directly set by whether the last received event for the key was a press or release.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> down;

        /// <summary>
        /// Was the key released this frame? This is not exclusive to pressed and both can occur during the same frame.
        /// </summary>
        std::bitset<static_cast<uint32_t>(Key::Count)> released;

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

    /// <summary>
    /// The current state of input for a frame.
    /// This is the raw state the simply records which buttons on currently down and does not discern between press, held, or release.
    /// </summary>
    struct InputState
    {
        KeyboardState keyboard{};

        void reset() noexcept
        {
            keyboard.reset();
        }
    };
}

#endif