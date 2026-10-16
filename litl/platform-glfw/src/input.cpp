#include "litl-platform-glfw/glfw.hpp"
#include "litl-platform-glfw/input.hpp"
#include "litl-platform-glfw/inputMappings.hpp"
#include "litl-core/logging/logging.hpp"

namespace litl
{
    struct InputContext
    {
        Input litlInput{};
        InputState frameState{};
    };

    InputContext& getGlfwInputContext() noexcept
    {
        static InputContext context{};
        return context;
    }
}

namespace litl::glfw
{
    // -------------------------------------------------------------------------------------
    // Creation
    // -------------------------------------------------------------------------------------

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

    // -------------------------------------------------------------------------------------
    // Input Ops Implementation
    // -------------------------------------------------------------------------------------

    void destroy(InputContext* context) noexcept
    {
        // ... todo ...
    }

    void poll(InputContext* context) noexcept
    {
        // ... no action in GLFW as polling is done by the window event loop ...
    }

    InputState getAccumulatedState(InputContext* context, bool reset) noexcept
    {
        InputState accumulated = context->frameState;
        context->frameState.reset();
        return accumulated;
    }

    // -------------------------------------------------------------------------------------
    // GLFW Events
    // https://www.glfw.org/docs/3.3/input_guide.html
    // -------------------------------------------------------------------------------------

    void onGlfwKeyEvent(InputContext* context, int key, int scancode, int action, int mods) noexcept
    {
        if (key < 0 || key > GLFW_KEY_LAST)
        {
            return;
        }

        const Key litlKey = g_GlfwKeyMap[key];

        if (litlKey == Key::Unknown)
        {
            return;
        }

        if (action == GLFW_PRESS)
        {
            context->frameState.keyboard.pressed[static_cast<uint32_t>(litlKey)] = true;
            context->frameState.keyboard.down[static_cast<uint32_t>(litlKey)] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            context->frameState.keyboard.released[static_cast<uint32_t>(litlKey)] = true;
            context->frameState.keyboard.down[static_cast<uint32_t>(litlKey)] = false;
        }
    }

    void onGlfwMouseButtonEvent(InputContext* context, int button, int action, int mods) noexcept
    {
        if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST)
        {
            return;
        }

        const MouseButton litlButton = g_GlfwMouseButtonMap[button];

        if (litlButton == MouseButton::Unknown)
        {
            return;
        }

        // ... todo ...
    }

    void onGlfwCursorPositionEvent(InputContext* context, double xpos, double ypos) noexcept
    {
        // ... todo ...
    }

    void onGlfwCursorEnterEvent(InputContext* context, int entered) noexcept
    {
        // ... todo ...
    }

    void onGlfwScrollEvent(InputContext* context, double xoffset, double yoffset) noexcept
    {
        // ... todo ...
    }
}