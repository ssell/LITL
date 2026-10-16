#ifndef LITL_PLATFORM_INPUT_ENUMS_H__
#define LITL_PLATFORM_INPUT_ENUMS_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Cursor capture modes.
    /// </summary>
    enum class CursorMode : uint8_t
    {
        Unknown        = 0u,
        Normal         = 1u,
        Hidden         = 2u,
        Captured       = 3u
    };

    /// <summary>
    /// Common mouse buttons.
    /// </summary>
    enum class MouseButton : uint8_t
    {
        Unknown        = 0u,
        Left           = 1u,
        Right          = 2u,
        Middle         = 3u,
        Forward        = 4u,
        Back           = 5u,
        // Number of buttons. Not a valid button.
        Count
    };

    /// <summary>
    /// Common gamepad buttons.
    /// </summary>
    enum class GamepadButton : uint8_t
    {
        Unknown        = 0u,        // Any button without a mapping.
        DPadUp         = 1u,        // The up button on the dpad.
        DPadDown       = 2u,        // The down button on the dpad.
        DPadLeft       = 3u,        // The left button on the dpad.
        DPadRight      = 4u,        // The right button on the dpad.
        North          = 5u,        // The upper action button on a gamepad.
        Y              = 5u,        // The upper action button on an Xbox controller.
        Triangle       = 5u,        // The upper action button on a Playstation controller.
        East           = 6u,        // The right action button on a gamepad.
        B              = 6u,        // The right action button on an Xbox controller.
        Circle         = 6u,        // The right action button on a Playstation controller.
        South          = 7u,        // The lower action button on a gamepad.
        A              = 7u,        // The lower action button on an Xbox controller.
        Cross          = 7u,        // The lower action button on a Playstation controller.
        West           = 8u,        // The left action button on a gamepad.
        X              = 8u,        // The left action button on an Xbox controller.
        Square         = 8u,        // The left action button on a Playstation controller.
        LeftStick      = 9u,        // The button pressed when pressing down the left stick on a gamepad.
        RightStick     = 10u,       // The button pressed when pressing down the right stick on a gamepad.
        LeftShoulder   = 11u,       // The left shoulder button on a gamepad.
        RightShoulder  = 12u,       // The right shoulder button on a gamepad.
        LeftTrigger    = 13u,       // The left trigger button on a gamepad.
        RightTrigger   = 14u,       // The right trigger button on a gamepad.
        Start          = 15u,       // The start button on a gamepad.
        Select         = 16u,       // The select button on a gamepad.
        // Number of buttons. Not a valid button.
        Count
    };

    /// <summary>
    /// Standard US keyboard keys, named after the physical key position on a US layout.
    /// Values are dense and start at 0 so that they can be used directly as indices (bitsets, arrays).
    /// Names follow System.Windows.Input.Key where practical, but the values do not.
    /// </summary>
    enum class Key : uint16_t
    {
        Unknown        = 0u,        // Any key without a mapping.
        // Letters
        A              = 1u,
        B              = 2u,
        C              = 3u,
        D              = 4u,
        E              = 5u,
        F              = 6u,
        G              = 7u,
        H              = 8u,
        I              = 9u,
        J              = 10u,
        K              = 11u,
        L              = 12u,
        M              = 13u,
        N              = 14u,
        O              = 15u,
        P              = 16u,
        Q              = 17u,
        R              = 18u,
        S              = 19u,
        T              = 20u,
        U              = 21u,
        V              = 22u,
        W              = 23u,
        X              = 24u,
        Y              = 25u,
        Z              = 26u,
        // Top-row digits
        D0             = 27u,
        D1             = 28u,
        D2             = 29u,
        D3             = 30u,
        D4             = 31u,
        D5             = 32u,
        D6             = 33u,
        D7             = 34u,
        D8             = 35u,
        D9             = 36u,
        // Function keys
        F1             = 37u,
        F2             = 38u,
        F3             = 39u,
        F4             = 40u,
        F5             = 41u,
        F6             = 42u,
        F7             = 43u,
        F8             = 44u,
        F9             = 45u,
        F10            = 46u,
        F11            = 47u,
        F12            = 48u,
        // Control
        Escape         = 49u,
        Tab            = 50u,
        CapsLock       = 51u,
        Space          = 52u,
        Enter          = 53u,
        Backspace      = 54u,
        PrintScreen    = 55u,
        ScrollLock     = 56u,
        Pause          = 57u,
        Apps           = 58u,       // The context menu key.
        // Modifiers
        LeftShift      = 59u,
        RightShift     = 60u,
        LeftCtrl       = 61u,
        RightCtrl      = 62u,
        LeftAlt        = 63u,
        RightAlt       = 64u,
        LeftSuper      = 65u,       // Left windows key
        RightSuper     = 66u,       // Right windows key
        // Navigation and editing
        Insert         = 67u,
        Delete         = 68u,
        Home           = 69u,
        End            = 70u,
        PageUp         = 71u,
        PageDown       = 72u,
        Left           = 73u,
        Up             = 74u,
        Right          = 75u,
        Down           = 76u,
        // Punctuation (US layout)
        Tilde          = 77u,       // ` ~
        Minus          = 78u,       // - _
        Equals         = 79u,       // = +
        LeftBracket    = 80u,       // [ {
        RightBracket   = 81u,       // ] }
        Backslash      = 82u,       // \ |
        Semicolon      = 83u,       // ; :
        Apostrophe     = 84u,       // ' "
        Comma          = 85u,       // , <
        Period         = 86u,       // . >
        Slash          = 87u,       // / ?
        // Numeric keypad
        NumLock        = 88u,
        NumPad0        = 89u,
        NumPad1        = 90u,
        NumPad2        = 91u,
        NumPad3        = 92u,
        NumPad4        = 93u,
        NumPad5        = 94u,
        NumPad6        = 95u,
        NumPad7        = 96u,
        NumPad8        = 97u,
        NumPad9        = 98u,
        NumPadDivide   = 99u,
        NumPadMultiply = 100u,
        NumPadSubtract = 101u,
        NumPadAdd      = 102u,
        NumPadDecimal  = 103u,
        NumPadEnter    = 104u,
        // Number of keys. Not a valid key.
        Count          = 105u
    };

    enum class KeyState : uint8_t
    {
        /// <summary>
        /// The default key state, when it has not been interacted with.
        /// </summary>
        None     = 0u,

        /// <summary>
        /// State of the key the exact frame it has been pressed down.
        /// </summary>
        Pressed  = 1u,

        /// <summary>
        /// State of the key if it is pressed down and this is not the first frame it was pressed down.
        /// </summary>
        Held     = 2u,

        /// <summary>
        /// State of the key the frame it is released.
        /// </summary>
        Released = 3u
    };
}

#endif