#ifndef LITL_PLATFORM_GLFW_PLATFORM_H__
#define LITL_PLATFORM_GLFW_PLATFORM_H__

#include "litl-platform/platform.hpp"

namespace litl
{
    namespace glfw
    {
        [[nodiscard]] bool initialize(PlatformContext* context) noexcept;
        [[nodiscard]] bool shutdown(PlatformContext* context) noexcept;
        [[nodiscard]] Window* createWindow(Platform* platform, PlatformContext* context) noexcept;
        [[nodiscard]] Window* getWindow(PlatformContext* context) noexcept;
        [[nodiscard]] Input* createInput(PlatformContext* context) noexcept;
        [[nodiscard]] Input* getInput(PlatformContext* context) noexcept;
    }

    inline constexpr PlatformOps GlfwPlatformOps = {
        .initialize = &glfw::initialize,
        .shutdown = &glfw::shutdown,
        .createWindow = &glfw::createWindow,
        .getWindow = &glfw::getWindow,
        .createInput = &glfw::createInput,
        .getInput = &glfw::getInput
    };
}

#endif