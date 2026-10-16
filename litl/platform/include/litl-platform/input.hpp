#ifndef LITL_PLATFORM_INPUT_H__
#define LITL_PLATFORM_INPUT_H__

#include "litl-platform/inputEnums.hpp"
#include "litl-platform/inputState.hpp"

namespace litl
{
    class Window;

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
        /// Returns the current state of a keyboard key.
        /// </summary>
        [[nodiscard]] KeyState getKeyState(Key key) const noexcept;

        /// <summary>
        /// Returns true if this is the first frame a key is held down (pressed).
        /// </summary>
        [[nodiscard]] bool getKeyPressed(Key key) const noexcept;

        /// <summary>
        /// Returns true if the keyboard key is held down and this is not the first frame (pressed).
        /// </summary>
        [[nodiscard]] bool getKeyHeld(Key key) const noexcept;

        /// <summary>
        /// Returns true the frame the keyboard key was released.
        /// </summary>
        [[nodiscard]] bool getKeyReleased(Key key) const noexcept;

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