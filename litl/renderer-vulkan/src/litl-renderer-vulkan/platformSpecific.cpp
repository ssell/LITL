#include <array>

#include "litl-core/assert.hpp"
#include "litl-core/constants.hpp"
#include "litl-renderer-vulkan/platformSpecific.hpp"
#include "litl-platform/window/window.hpp"

// -----------------------------------------------------------------------------------------
// Windows
// -----------------------------------------------------------------------------------------

#if defined(LITL_WINDOWS)

// Defined in the CMakeLists.txt already: VK_USE_PLATFORM_WIN32_KHR
#include "litl-renderer-vulkan/rendererContext.hpp"

namespace litl::vulkan
{
    bool createVkSurface(RendererContext& context) noexcept
    {
        const auto nativeWindowHandle = context.window.window->getNativeWindowHandle();
        LITL_FATAL_ASSERT(nativeWindowHandle.kind == NativeWindowKind::Win32);

        const VkWin32SurfaceCreateInfoKHR surfaceInfo{
            .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = *static_cast<HINSTANCE const*>(nativeWindowHandle.display),
            .hwnd = *static_cast<HWND const*>(nativeWindowHandle.window)
        };

        const auto result = vkCreateWin32SurfaceKHR(context.device.vkInstance, &surfaceInfo, nullptr, &context.device.vkSurface);

        if (result != VK_SUCCESS)
        {
            return false;
        }

        return true;
    }

    constexpr uint32_t WindowsRequiredExtensionsCount = 2u;
    constexpr const char* const WindowsRequiredExtensions[WindowsRequiredExtensionsCount]{
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };

    const char* const* getRequiredPlatformExtensions(uint32_t& extensionsCount) noexcept
    {
        extensionsCount = WindowsRequiredExtensionsCount;
        return WindowsRequiredExtensions;
    }
}

// -----------------------------------------------------------------------------------------
// Linux
// -----------------------------------------------------------------------------------------

#elif defined(LITL_LINUX)

// ... todo ...

// -----------------------------------------------------------------------------------------
// MacOS
// -----------------------------------------------------------------------------------------

#elif defined(LITL_MACOS)

    // ... todo ...

// -----------------------------------------------------------------------------------------
// Other
// -----------------------------------------------------------------------------------------

#else
    // ... todo ...
#endif