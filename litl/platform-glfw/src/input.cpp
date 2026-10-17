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
        // Most input polling is done by the glfwPollEvents in the window, except for gamepad input.
        // That is a ... todo ... and uses glfwGetGamepadState which will be done in here.
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

        if (action == GLFW_PRESS)
        {
            context->frameState.mouse.pressed[static_cast<uint32_t>(litlButton)] = true;
            context->frameState.mouse.down[static_cast<uint32_t>(litlButton)] = true;
        }
        else if (action == GLFW_RELEASE)
        {
            context->frameState.mouse.released[static_cast<uint32_t>(litlButton)] = true;
            context->frameState.mouse.down[static_cast<uint32_t>(litlButton)] = false;
        }
    }

    void onGlfwCursorPositionEvent(InputContext* context, double xpos, double ypos) noexcept
    {
        context->frameState.mouse.position.xy = vec2{ static_cast<float>(xpos), static_cast<float>(ypos) };
    }

    void onGlfwCursorEnterEvent(InputContext* context, int entered) noexcept
    {
        if (entered != 0)
        {
            context->frameState.mouse.position.enteredWindow = true;
            context->frameState.mouse.position.inWindow = true;
        }
        else
        {
            context->frameState.mouse.position.exitedWindow = true;
            context->frameState.mouse.position.inWindow = false;
        }
    }

    void onGlfwScrollEvent(InputContext* context, double xoffset, double yoffset) noexcept
    {
        context->frameState.mouse.scroll += vec2{ static_cast<float>(xoffset), static_cast<float>(yoffset) };
    }
}