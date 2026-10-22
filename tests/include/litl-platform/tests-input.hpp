#ifndef LITL_TESTS_PLATFORM_INPUT_H__
#define LITL_TESTS_PLATFORM_INPUT_H__

#include <memory>
#include <optional>

#include "tests.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-platform/platform.hpp"
#include "litl-platform/input/input.hpp"
#include "litl-platform/input/inputConfiguration.hpp"

namespace litl::tests
{
    /// <summary>
    /// Headless input backend. Tests write events into `next`, which the following Input::poll() consumes.
    /// Mirrors the GLFW accumulator: pressed/released/enter/exit/scroll are cleared after each poll, while
    /// down state, cursor position, and cursor mode persist.
    /// </summary>
    struct FakeInputBackend
    {
        InputState next{};
        CursorMode cursorMode{ CursorMode::Normal };
        uint32_t setCursorModeCalls{ 0u };

        void pressKey(Key key) noexcept
        {
            const auto i = static_cast<uint32_t>(key);
            next.keyboard.pressed.set(i);
            next.keyboard.down.set(i);
        }

        void releaseKey(Key key) noexcept
        {
            const auto i = static_cast<uint32_t>(key);
            next.keyboard.released.set(i);
            next.keyboard.down.reset(i);
        }

        void tapKey(Key key) noexcept
        {
            pressKey(key);
            releaseKey(key);
        }

        void pressMouseButton(MouseButton button) noexcept
        {
            const auto i = static_cast<uint32_t>(button);
            next.mouse.pressed.set(i);
            next.mouse.down.set(i);
        }

        void releaseMouseButton(MouseButton button) noexcept
        {
            const auto i = static_cast<uint32_t>(button);
            next.mouse.released.set(i);
            next.mouse.down.reset(i);
        }

        void moveMouseTo(float x, float y) noexcept
        {
            next.mouse.position.xy = vec2{ x, y };
            next.mouse.position.inWindow = true;
        }

        void enterWindowAt(float x, float y) noexcept
        {
            moveMouseTo(x, y);
            next.mouse.position.enteredWindow = true;
        }

        void scroll(float x, float y) noexcept
        {
            next.mouse.scroll = next.mouse.scroll + vec2{ x, y };
        }

        [[nodiscard]] InputContext* context() noexcept
        {
            return reinterpret_cast<InputContext*>(this);
        }

        [[nodiscard]] static FakeInputBackend& from(InputContext* context) noexcept
        {
            return *reinterpret_cast<FakeInputBackend*>(context);
        }
    };

    namespace fakeInput
    {
        inline void destroy(InputContext*) noexcept {}
        inline void setActiveWindow(InputContext*, Window*) noexcept {}
        inline void poll(InputContext*) noexcept {}

        inline InputState getAccumulatedState(InputContext* context, bool) noexcept
        {
            auto& backend = FakeInputBackend::from(context);
            InputState accumulated = backend.next;
            backend.next.reset();
            return accumulated;
        }

        inline void setCursorMode(InputContext* context, CursorMode mode) noexcept
        {
            auto& backend = FakeInputBackend::from(context);
            backend.cursorMode = mode;
            backend.next.mouse.mode = mode;
            ++backend.setCursorModeCalls;
        }
    }

    inline constexpr InputOps FakeInputOps{
        .destroy = &fakeInput::destroy,
        .setActiveWindow = &fakeInput::setActiveWindow,
        .poll = &fakeInput::poll,
        .getAccumulatedState = &fakeInput::getAccumulatedState,
        .setCursorMode = &fakeInput::setCursorMode
    };

    struct InputFixture
    {
        FakeInputBackend backend{};
        Input input{};
        ServiceCollection collection{};
        std::shared_ptr<ServiceProvider> services{ collection.build() };
        InputCallbacks callbacks{};

        void start(InputConfiguration const& config = {})
        {
            REQUIRE(input.setup(&FakeInputOps, backend.context()));
            input.prepare(Authority<Platform>::mint(), config);
        }

        /// <summary>
        /// One engine frame worth of input: poll the backend, then dispatch callbacks.
        /// </summary>
        void frame()
        {
            input.poll();
            input.triggerCallbacks(*services, callbacks);
        }

        [[nodiscard]] ActionMapState action(StringId map, StringId binding)
        {
            const auto state = input.getActionState(input.getMap(map), binding);
            REQUIRE(state.has_value());
            return state.value_or(ActionMapState{});
        }
    };
}

#endif