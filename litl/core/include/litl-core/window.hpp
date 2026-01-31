#ifndef LITL_CORE_WINDOW_H__
#define LITL_CORE_WINDOW_H__

#include <cstdint>

namespace LITL::Core
{
    enum WindowState
    {
        Open = 0,
        Minimized = 1
    };

    class Window
    {
    public:

        virtual ~Window() = default;

        virtual bool open(const char* title, uint32_t width, uint32_t height) = 0;
        virtual bool close() = 0;
        virtual bool shouldClose() = 0;

        virtual WindowState getState() const = 0;
        virtual uint32_t getWidth() const = 0;
        virtual uint32_t getHeight() const = 0;

    protected:

        virtual void onResize(uint32_t width, uint32_t height) = 0;

    private:
    };
}

#endif