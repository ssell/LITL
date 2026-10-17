#ifndef LITL_PLATFORM_GLFW_GLFW_H__
#define LITL_PLATFORM_GLFW_GLFW_H__

#include "litl-core/constants.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#if defined(LITL_WINDOWS)
    #define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(LITL_LINUX)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_GLX
#elif defined(LITL_MACOS)
    #define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include <GLFW/glfw3native.h>

#endif