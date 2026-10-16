#ifndef LITL_PLATFORM_INPUT_H__
#define LITL_PLATFORM_INPUT_H__

#include "litl-platform/inputEnums.hpp"
#include "litl-platform/inputState.hpp"
#include "litl-platform/inputCallbacks.hpp"

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
        /// Returns ture if the key was released since the last poll (typically the last frame).
        /// The "released" state is not exclusive with the "pressed" state, and a key can be both pressed and released during the same frame.
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