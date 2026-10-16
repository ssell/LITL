#ifndef LITL_PLATFORM_GLFW_INPUT_MAPPINGS_H__
#define LITL_PLATFORM_GLFW_INPUT_MAPPINGS_H__

#include <array>

#include "litl-platform/inputEnums.hpp"
#include "litl-platform-glfw/glfw.hpp"

namespace litl::glfw
{
    inline constexpr std::array<Key, GLFW_KEY_LAST + 1> g_GlfwKeyMap
    {
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Space,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Apostrophe,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Comma,
        Key::Minus,
        Key::Period,
        Key::Slash,
        Key::D0,
        Key::D1,
        Key::D2,
        Key::D3,
        Key::D4,
        Key::D5,
        Key::D6,
        Key::D7,
        Key::D8,
        Key::D9,
        Key::Unknown,
        Key::Semicolon,
        Key::Unknown,
        Key::Equals,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::A,
        Key::B,
        Key::C,
        Key::D,
        Key::E,
        Key::F,
        Key::G,
        Key::H,
        Key::I,
        Key::J,
        Key::K,
        Key::L,
        Key::M,
        Key::N,
        Key::O,
        Key::P,
        Key::Q,
        Key::R,
        Key::S,
        Key::T,
        Key::U,
        Key::V,
        Key::W,
        Key::X,
        Key::Y,
        Key::Z,
        Key::LeftBracket,
        Key::Backslash,
        Key::RightBracket,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Tilde,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Escape,
        Key::Enter,
        Key::Tab,
        Key::Backspace,
        Key::Insert,
        Key::Delete,
        Key::Right,
        Key::Left,
        Key::Down,
        Key::Up,
        Key::PageUp,
        Key::PageDown,
        Key::Home,
        Key::End,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::CapsLock,
        Key::ScrollLock,
        Key::NumLock,
        Key::PrintScreen,
        Key::Pause,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::F1,
        Key::F2,
        Key::F3,
        Key::F4,
        Key::F5,
        Key::F6,
        Key::F7,
        Key::F8,
        Key::F9,
        Key::F10,
        Key::F11,
        Key::F12,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::NumPad0,
        Key::NumPad1,
        Key::NumPad2,
        Key::NumPad3,
        Key::NumPad4,
        Key::NumPad5,
        Key::NumPad6,
        Key::NumPad7,
        Key::NumPad8,
        Key::NumPad9,
        Key::NumPadDecimal,
        Key::NumPadDivide,
        Key::NumPadMultiply,
        Key::NumPadSubtract,
        Key::NumPadAdd,
        Key::NumPadEnter,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::Unknown,
        Key::LeftShift,
        Key::LeftCtrl,
        Key::LeftAlt,
        Key::LeftSuper,
        Key::RightShift,
        Key::RightCtrl,
        Key::RightAlt,
        Key::RightSuper,
        Key::Unknown
    };

    static_assert(g_GlfwKeyMap[GLFW_KEY_SPACE]          == Key::Space);
    static_assert(g_GlfwKeyMap[GLFW_KEY_APOSTROPHE]     == Key::Apostrophe);
    static_assert(g_GlfwKeyMap[GLFW_KEY_COMMA]          == Key::Comma);
    static_assert(g_GlfwKeyMap[GLFW_KEY_MINUS]          == Key::Minus);
    static_assert(g_GlfwKeyMap[GLFW_KEY_PERIOD]         == Key::Period);
    static_assert(g_GlfwKeyMap[GLFW_KEY_SLASH]          == Key::Slash);
    static_assert(g_GlfwKeyMap[GLFW_KEY_0]              == Key::D0);
    static_assert(g_GlfwKeyMap[GLFW_KEY_1]              == Key::D1);
    static_assert(g_GlfwKeyMap[GLFW_KEY_2]              == Key::D2);
    static_assert(g_GlfwKeyMap[GLFW_KEY_3]              == Key::D3);
    static_assert(g_GlfwKeyMap[GLFW_KEY_4]              == Key::D4);
    static_assert(g_GlfwKeyMap[GLFW_KEY_5]              == Key::D5);
    static_assert(g_GlfwKeyMap[GLFW_KEY_6]              == Key::D6);
    static_assert(g_GlfwKeyMap[GLFW_KEY_7]              == Key::D7);
    static_assert(g_GlfwKeyMap[GLFW_KEY_8]              == Key::D8);
    static_assert(g_GlfwKeyMap[GLFW_KEY_9]              == Key::D9);
    static_assert(g_GlfwKeyMap[GLFW_KEY_SEMICOLON]      == Key::Semicolon);
    static_assert(g_GlfwKeyMap[GLFW_KEY_EQUAL]          == Key::Equals);
    static_assert(g_GlfwKeyMap[GLFW_KEY_A]              == Key::A);
    static_assert(g_GlfwKeyMap[GLFW_KEY_B]              == Key::B);
    static_assert(g_GlfwKeyMap[GLFW_KEY_C]              == Key::C);
    static_assert(g_GlfwKeyMap[GLFW_KEY_D]              == Key::D);
    static_assert(g_GlfwKeyMap[GLFW_KEY_E]              == Key::E);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F]              == Key::F);
    static_assert(g_GlfwKeyMap[GLFW_KEY_G]              == Key::G);
    static_assert(g_GlfwKeyMap[GLFW_KEY_H]              == Key::H);
    static_assert(g_GlfwKeyMap[GLFW_KEY_I]              == Key::I);
    static_assert(g_GlfwKeyMap[GLFW_KEY_J]              == Key::J);
    static_assert(g_GlfwKeyMap[GLFW_KEY_K]              == Key::K);
    static_assert(g_GlfwKeyMap[GLFW_KEY_L]              == Key::L);
    static_assert(g_GlfwKeyMap[GLFW_KEY_M]              == Key::M);
    static_assert(g_GlfwKeyMap[GLFW_KEY_N]              == Key::N);
    static_assert(g_GlfwKeyMap[GLFW_KEY_O]              == Key::O);
    static_assert(g_GlfwKeyMap[GLFW_KEY_P]              == Key::P);
    static_assert(g_GlfwKeyMap[GLFW_KEY_Q]              == Key::Q);
    static_assert(g_GlfwKeyMap[GLFW_KEY_R]              == Key::R);
    static_assert(g_GlfwKeyMap[GLFW_KEY_S]              == Key::S);
    static_assert(g_GlfwKeyMap[GLFW_KEY_T]              == Key::T);
    static_assert(g_GlfwKeyMap[GLFW_KEY_U]              == Key::U);
    static_assert(g_GlfwKeyMap[GLFW_KEY_V]              == Key::V);
    static_assert(g_GlfwKeyMap[GLFW_KEY_W]              == Key::W);
    static_assert(g_GlfwKeyMap[GLFW_KEY_X]              == Key::X);
    static_assert(g_GlfwKeyMap[GLFW_KEY_Y]              == Key::Y);
    static_assert(g_GlfwKeyMap[GLFW_KEY_Z]              == Key::Z);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT_BRACKET]   == Key::LeftBracket);
    static_assert(g_GlfwKeyMap[GLFW_KEY_BACKSLASH]      == Key::Backslash);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT_BRACKET]  == Key::RightBracket);
    static_assert(g_GlfwKeyMap[GLFW_KEY_GRAVE_ACCENT]   == Key::Tilde);
    static_assert(g_GlfwKeyMap[GLFW_KEY_ESCAPE]         == Key::Escape);
    static_assert(g_GlfwKeyMap[GLFW_KEY_ENTER]          == Key::Enter);
    static_assert(g_GlfwKeyMap[GLFW_KEY_TAB]            == Key::Tab);
    static_assert(g_GlfwKeyMap[GLFW_KEY_BACKSPACE]      == Key::Backspace);
    static_assert(g_GlfwKeyMap[GLFW_KEY_INSERT]         == Key::Insert);
    static_assert(g_GlfwKeyMap[GLFW_KEY_DELETE]         == Key::Delete);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT]          == Key::Right);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT]           == Key::Left);
    static_assert(g_GlfwKeyMap[GLFW_KEY_DOWN]           == Key::Down);
    static_assert(g_GlfwKeyMap[GLFW_KEY_UP]             == Key::Up);
    static_assert(g_GlfwKeyMap[GLFW_KEY_PAGE_UP]        == Key::PageUp);
    static_assert(g_GlfwKeyMap[GLFW_KEY_PAGE_DOWN]      == Key::PageDown);
    static_assert(g_GlfwKeyMap[GLFW_KEY_HOME]           == Key::Home);
    static_assert(g_GlfwKeyMap[GLFW_KEY_END]            == Key::End);
    static_assert(g_GlfwKeyMap[GLFW_KEY_CAPS_LOCK]      == Key::CapsLock);
    static_assert(g_GlfwKeyMap[GLFW_KEY_SCROLL_LOCK]    == Key::ScrollLock);
    static_assert(g_GlfwKeyMap[GLFW_KEY_NUM_LOCK]       == Key::NumLock);
    static_assert(g_GlfwKeyMap[GLFW_KEY_PRINT_SCREEN]   == Key::PrintScreen);
    static_assert(g_GlfwKeyMap[GLFW_KEY_PAUSE]          == Key::Pause);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F1]             == Key::F1);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F2]             == Key::F2);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F3]             == Key::F3);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F4]             == Key::F4);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F5]             == Key::F5);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F6]             == Key::F6);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F7]             == Key::F7);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F8]             == Key::F8);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F9]             == Key::F9);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F10]            == Key::F10);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F11]            == Key::F11);
    static_assert(g_GlfwKeyMap[GLFW_KEY_F12]            == Key::F12);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_0]           == Key::NumPad0);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_1]           == Key::NumPad1);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_2]           == Key::NumPad2);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_3]           == Key::NumPad3);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_4]           == Key::NumPad4);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_5]           == Key::NumPad5);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_6]           == Key::NumPad6);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_7]           == Key::NumPad7);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_8]           == Key::NumPad8);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_9]           == Key::NumPad9);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_DECIMAL]     == Key::NumPadDecimal);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_DIVIDE]      == Key::NumPadDivide);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_MULTIPLY]    == Key::NumPadMultiply);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_SUBTRACT]    == Key::NumPadSubtract);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_ADD]         == Key::NumPadAdd);
    static_assert(g_GlfwKeyMap[GLFW_KEY_KP_ENTER]       == Key::NumPadEnter);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT_SHIFT]     == Key::LeftShift);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT_CONTROL]   == Key::LeftCtrl);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT_ALT]       == Key::LeftAlt);
    static_assert(g_GlfwKeyMap[GLFW_KEY_LEFT_SUPER]     == Key::LeftSuper);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT_SHIFT]    == Key::RightShift);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT_CONTROL]  == Key::RightCtrl);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT_ALT]      == Key::RightAlt);
    static_assert(g_GlfwKeyMap[GLFW_KEY_RIGHT_SUPER]    == Key::RightSuper);
}

#endif