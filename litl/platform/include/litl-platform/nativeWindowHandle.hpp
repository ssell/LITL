#ifndef LITL_PLATFORM_NATIVE_WINDOW_HANDLE_H__
#define LITL_PLATFORM_NATIVE_WINDOW_HANDLE_H__

#include <cstdint>

namespace litl
{
    enum class NativeWindowKind : uint8_t
    {
        Unknown = 0u,
        Win32 = 1u,
        XLib = 2u,
        Wayland = 3u,
        Cocoa = 4u,
        HtmlCanvas = 5u
    };

    struct NativeWindowHandle
    {
        /// <summary>
        /// The type of underlying native window.
        /// </summary>
        NativeWindowKind kind{ NativeWindowKind::Unknown };

        /// <summary>
        /// Win32 = HWND, 
        /// Xlib = Window (an XID cast to void*), 
        /// Wayland = wl_surface*, 
        /// Cocoa = NSView* or CAMetalLayer*, 
        /// HtmlCanvas = unused (not Vulkan compatible, forward facing for potential WebGPU backend)
        /// </summary>
        void const* window{ nullptr };

        /// <summary>
        /// Win32 = HINSTANCE,
        /// Xlib = Display*,
        /// Wayland = wl_display*,
        /// Cocoa = unused,
        /// HtmlCanvas = unused (not Vulkan compatible, forward facing for potential WebGPU backend)
        /// </summary>
        void const* display{ nullptr };
    };
}

#endif