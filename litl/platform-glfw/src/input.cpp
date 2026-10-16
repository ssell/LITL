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

    // -------------------------------------------------------------------------------------
    // GLFW Events
    // -------------------------------------------------------------------------------------

    namespace
    {
        
        constexpr std::array<MouseButton, GLFW_MOUSE_BUTTON_LAST + 1> g_GlfwMouseButtonMap{};
        constexpr std::array<GamepadButton, GLFW_GAMEPAD_BUTTON_LAST + 1> g_GlfwGamepadButtonMap{};
    }

    void onGlfwKeyEvent(InputContext* context, int key, int scancode, int action, int mods) noexcept
    {
        // ... todo ...
    }

    void onGlfwMouseButtonEvent(InputContext* context, int button, int action, int mods) noexcept
    {
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