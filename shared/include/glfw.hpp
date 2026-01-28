#ifndef LITL_GLFW_H__
#define LITL_GLFW_H__

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>       // For surface creation
#include <functional>
#include "demoProgram.hpp"

typedef void (*onWindowResize)(int width, int height);

GLFWwindow* createGlfwWindow(int width, int height, const char* title, void* application)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    auto pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!pWindow)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwSetWindowUserPointer(pWindow, application);
    glfwSetFramebufferSizeCallback(pWindow, [](GLFWwindow* window, int width, int height)
        {
            auto* renderer = static_cast<LITL::DemoProgram*>(glfwGetWindowUserPointer(window));
            renderer->onResize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
        });

    return pWindow;
}

void closeGlfwWindow(GLFWwindow* pWindow)
{
    if (pWindow != nullptr)
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }
}

#endif
