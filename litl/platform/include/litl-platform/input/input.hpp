#ifndef LITL_PLATFORM_INPUT_H__
#define LITL_PLATFORM_INPUT_H__

#include "litl-platform/input/inputEnums.hpp"
#include "litl-platform/input/inputState.hpp"
#include "litl-platform/input/inputCallbacks.hpp"
#include "litl-platform/input/inputSource.hpp"

namespace litl
{
    class Window;
    class ServiceProvider;

    /// <summary>
    /// Explicitly not defined in litl-platform, but instead defined in the implementation (litl-platform-glfw, etc.)
    /// </summary>
    struct InputContext;

    struct InputOps
    {
        void (*destroy)(InputContext*);
        void (*poll)(InputContext*);
        InputState (*getAccumulatedState)(InputContext*, bool);
    };

    class Input final
    {
    public:

        Input();
        ~Input();

        Input(Input const&) = delete;
        Input& operator=(Input const&) = delete;

        /// <summary>
        /// Sets up the input interface with a backend.
        /// </summary>
        [[nodiscard]] bool setup(InputOps const* operations, InputContext* context) noexcept;

        /// <summary>
        /// Destroys the underlying input backend.
        /// </summary>
        void destroy() noexcept;

        /// <summary>
        /// Polls for new input events. Must be called once per frame from the main thread.
        /// </summary>
        void poll() noexcept;

        /// <summary>
        /// 
        /// </summary>
        void triggerCallbacks(ServiceProvider& services, InputCallbacks& callbacks) const noexcept;

        /// <summary>
        /// Returns true if the key was pressed down since the last poll (typically the last frame).
        /// The "pressed" state is not exclusive with the "released" state, and a key can be both pressed and released during the same frame.
        /// </summary>
        [[nodiscard]] bool getKeyPressed(Key key) const noexcept;

        /// <summary>
        /// Returns true if the key was currently down at the last poll.
        /// A key that is down but was not pressed this frame can be considered "held down", while a key that is not down
        /// but was pressed and released this frame can be interpreted as a rapid single-frame tap.
        /// </summary>
        [[nodiscard]] bool getKeyHeld(Key key) const noexcept;

        /// <summary>
        /// Returns true if the key was released since the last poll (typically the last frame).
        /// The "released" state is not exclusive with the "pressed" state, and a key can be both pressed and released during the same frame.
        /// </summary>
        [[nodiscard]] bool getKeyReleased(Key key) const noexcept;

        /// <summary>
        /// Returns true if the button was pressed down since the last poll (typically the last frame).
        /// The "pressed" state is not exclusive with the "released" state, and a button can be both pressed and released during the same frame.
        /// </summary>
        [[nodiscard]] bool getMouseButtonPressed(MouseButton button) const noexcept;

        /// <summary>
        /// Returns true if the button was currently down at the last poll.
        /// A button that is down but was not pressed this frame can be considered "held down", while a button that is not down
        /// but was pressed and released this frame can be interpreted as a rapid single-frame tap.
        /// </summary>
        [[nodiscard]] bool getMouseButtonHeld(MouseButton button) const noexcept;

        /// <summary>
        /// Returns true if the button was released since the last poll (typically the last frame).
        /// The "released" state is not exclusive with the "pressed" state, and a button can be both pressed and released during the same frame.
        /// </summary>
        [[nodiscard]] bool getMouseButtonReleased(MouseButton button) const noexcept;

        /// <summary>
        /// Returns the current mouse position, delta, and if it is in the window.
        /// </summary>
        [[nodiscard]] MousePosition const& getMousePosition() const noexcept;

        /// <summary>
        /// Returns true if the mouse entered the window since the last poll.
        /// Note that this is not exclusive to the mouse exiting. Both can occur on the same frame.
        /// </summary>
        [[nodiscard]] bool getMouseEntered() const noexcept;

        /// <summary>
        /// Returns true if the mouse existed the window since the last poll.
        /// Note that this is not exclusive to the mouse entering. Both can occur on the same frame.
        /// </summary>
        [[nodiscard]] bool getMouseExited() const noexcept;

        /// <summary>
        /// Is the mouse in the window as of the last poll?
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] bool getMouseInWindow() const noexcept;

        /// <summary>
        /// Returns the xy-scroll amount since the last poll.
        /// 
        /// .x = horizontal scroll amount, which is typically 0.
        /// .y = vertical scroll amout, where positive is up/forward and negative is down/backward.
        /// 
        /// A single "tick" of the scroll wheel is typically a value of +/- 1. A particularly fast scroll,
        /// or particularly slow frame, may show greater magnitude values.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] vec2 getMouseScroll() const noexcept;

        [[nodiscard]] bool getGamepadButtonPressed(GamepadButton button) const noexcept;
        [[nodiscard]] bool getGamepadButtonHeld(GamepadButton button) const noexcept;
        [[nodiscard]] bool getGamepadButtonReleased(GamepadButton button) const noexcept;

        /// <summary>
        /// Given a generic InputSource, returns the state of the input.
        /// </summary>
        [[nodiscard]] InputSourceState getGenericInputSourceState(InputSource inputSource) const noexcept;

        /// <summary>
        /// Returns the internal context.
        /// </summary>
        [[nodiscard]] InputContext* getContext() noexcept;

    private:

        InputOps const* m_pOps{ nullptr };
        InputContext* m_pContext{ nullptr };
        InputState m_lastFrameState{};
        InputState m_currFrameState{};
    };
}

#endif