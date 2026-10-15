#include "litl-platform-glfw/input.hpp"
#include "litl-platform-glfw/glfw.hpp"
#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    struct InputContext
    {
        Input litlInput{};
    };

    InputContext& getGlfwInputContext() noexcept
    {
        static InputContext context{};
        return context;
    }
}

namespace litl::glfw
{
    Input* createGlfwInput() noexcept
    {
        InputContext& context = getGlfwInputContext();

        if (!context.litlInput.setup(&GlfwInputOps, &context))
        {
            logError("Failed to setup GLFW input.");
            return nullptr;
        }

        return &context.litlInput;
    }

    void destroy(InputContext* context) noexcept
    {
        // ... todo ...
    }

    void poll(InputContext* context) noexcept
    {
        // ... todo ...
    }

    KeyState getKeyState(InputContext* context, Key key) noexcept
    {
        // ... todo ...
        return KeyState::None;
    }

    bool getKeyPressed(InputContext* context, Key key) noexcept
    {
        // ... todo ...
        return false;
    }

    bool getKeyHeld(InputContext* context, Key key) noexcept
    {
        // ... todo ...
        return false;
    }

    bool getKeyReleased(InputContext* context, Key key) noexcept
    {
        // ... todo ...
        return false;
    }
}