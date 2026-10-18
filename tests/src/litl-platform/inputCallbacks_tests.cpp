#include "tests.hpp"
#include "litl-core/services/serviceCollection.hpp"
#include "litl-platform/inputCallbacks.hpp"

namespace litl::tests
{
    namespace
    {
        struct InputCallbacksFixture
        {
            ServiceCollection collection{};
            std::shared_ptr<ServiceProvider> services{ collection.build() };
            InputCallbacks callbacks{};

            void process()
            {
                callbacks.processDeferredCommands(Authority<Input>::mint());
            }

            void pressKey(Key key)
            {
                callbacks.invokeKeyEvent(Authority<Input>::mint(), *services, key, true, false, true);
            }

            void clickMouse(MouseButton button)
            {
                callbacks.invokeMouseButtonEvent(Authority<Input>::mint(), *services, MousePosition{}, button, true, false, true);
            }

            void moveMouse()
            {
                callbacks.invokeMouseMoveEvent(Authority<Input>::mint(), *services, MousePosition{ .xyDelta = vec2{ 1.0f, 0.0f } });
            }

            void scrollMouse()
            {
                callbacks.invokeMouseScrollEvent(Authority<Input>::mint(), *services, MousePosition{}, vec2{ 0.0f, 1.0f });
            }

            void enterWindow()
            {
                callbacks.invokeMouseEnterExitEvent(Authority<Input>::mint(), *services, MousePosition{ .enteredWindow = true, .inWindow = true });
            }
        };

        /// <summary>
        /// Key callback that increments a counter and returns the specified result.
        /// </summary>
        OnKeyFunc countingKeyCallback(uint32_t& counter, InputCallbackResult result)
        {
            return [&counter, result](ServiceProvider&, Key, bool, bool, bool)
            {
                ++counter;
                return result;
            };
        }
    }

    // -------------------------------------------------------------------------------------
    // Registration
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "registration is deferred until processed", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));

        REQUIRE(handle.id != Constants::uint32_null_index);
        REQUIRE(handle.type == InputCallbackType::Key);
        REQUIRE(handle.slot == static_cast<uint32_t>(Key::A));

        pressKey(Key::A);
        REQUIRE(count == 0u);

        process();
        pressKey(Key::A);
        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "registering an unknown key returns a null handle", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerKeyCallback(Key::Unknown, countingKeyCallback(count, InputCallbackResult::Continue));

        REQUIRE(handle.type == InputCallbackType::Unknown);
        REQUIRE(handle.id == Constants::uint32_null_index);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "handles are unique", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto a = callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));
        const auto b = callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));

        REQUIRE(a.id != b.id);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "key callbacks only fire for their key", "[platform::inputCallbacks]")
    {
        uint32_t countA = 0u;
        uint32_t countB = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(countA, InputCallbackResult::Continue));
        callbacks.registerKeyCallback(Key::B, countingKeyCallback(countB, InputCallbackResult::Continue));
        process();

        pressKey(Key::A);

        REQUIRE(countA == 1u);
        REQUIRE(countB == 0u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "events without a change are not dispatched", "[platform::inputCallbacks]")
    {
        uint32_t keyCount = 0u;
        uint32_t moveCount = 0u;
        uint32_t scrollCount = 0u;
        uint32_t enterExitCount = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(keyCount, InputCallbackResult::Continue));
        callbacks.registerMouseMoveCallback([&moveCount](ServiceProvider&, MousePosition const&) { ++moveCount; return InputCallbackResult::Continue; });
        callbacks.registerMouseScrollCallback([&scrollCount](ServiceProvider&, MousePosition const&, vec2) { ++scrollCount; return InputCallbackResult::Continue; });
        callbacks.registerMouseEnterExitCallback([&enterExitCount](ServiceProvider&, MousePosition const&) { ++enterExitCount; return InputCallbackResult::Continue; });
        process();

        const auto auth = Authority<Input>::mint();

        callbacks.invokeKeyEvent(auth, *services, Key::A, false, false, true);             // held, no edge
        callbacks.invokeMouseMoveEvent(auth, *services, MousePosition{});                  // zero delta
        callbacks.invokeMouseScrollEvent(auth, *services, MousePosition{}, vec2{});        // zero scroll
        callbacks.invokeMouseEnterExitEvent(auth, *services, MousePosition{});             // neither entered nor exited

        REQUIRE(keyCount == 0u);
        REQUIRE(moveCount == 0u);
        REQUIRE(scrollCount == 0u);
        REQUIRE(enterExitCount == 0u);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Propagation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "continue propagates to all callbacks", "[platform::inputCallbacks]")
    {
        uint32_t first = 0u;
        uint32_t second = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(first, InputCallbackResult::Continue));
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(second, InputCallbackResult::Continue));
        process();

        pressKey(Key::A);

        REQUIRE(first == 1u);
        REQUIRE(second == 1u);      // bug #1: currently 0
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "consume stops propagation", "[platform::inputCallbacks]")
    {
        uint32_t first = 0u;
        uint32_t second = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(first, InputCallbackResult::Consume));
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(second, InputCallbackResult::Continue));
        process();

        pressKey(Key::A);

        REQUIRE(first == 1u);
        REQUIRE(second == 0u);      // bug #1: currently 1
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "callbacks are invoked in registration order", "[platform::inputCallbacks]")
    {
        uint32_t order[2]{ 0u, 0u };
        uint32_t counter = 0u;

        callbacks.registerKeyCallback(Key::A, [&order, &counter](ServiceProvider&, Key, bool, bool, bool) { order[0] = ++counter; return InputCallbackResult::Continue; });
        callbacks.registerKeyCallback(Key::A, [&order, &counter](ServiceProvider&, Key, bool, bool, bool) { order[1] = ++counter; return InputCallbackResult::Continue; });
        process();

        pressKey(Key::A);

        REQUIRE(order[0] == 1u);
        REQUIRE(order[1] == 2u);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Self-removal
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "continue and remove fires once and still propagates", "[platform::inputCallbacks]")
    {
        uint32_t oneShot = 0u;
        uint32_t persistent = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(oneShot, InputCallbackResult::ContinueAndRemove));
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(persistent, InputCallbackResult::Continue));
        process();

        pressKey(Key::A);

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 1u);

        process();
        pressKey(Key::A);

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 2u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "consume and remove fires once and stops propagation for that frame", "[platform::inputCallbacks]")
    {
        uint32_t oneShot = 0u;
        uint32_t persistent = 0u;

        callbacks.registerKeyCallback(Key::A, countingKeyCallback(oneShot, InputCallbackResult::ConsumeAndRemove));
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(persistent, InputCallbackResult::Continue));
        process();

        pressKey(Key::A);

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 0u);

        process();
        pressKey(Key::A);

        REQUIRE(oneShot == 1u);
        REQUIRE(persistent == 1u);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Unregistration
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "unregister key callback", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));
        process();

        callbacks.unregisterCallback(handle);
        process();
        pressKey(Key::A);

        REQUIRE(count == 0u);       // bug #2: handle has type MouseButton, so the key callback is never removed
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "unregister mouse button callback", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerMouseButtonCallback(MouseButton::Left, [&count](ServiceProvider&, MousePosition const&, MouseButton, bool, bool, bool)
        {
            ++count;
            return InputCallbackResult::Continue;
        });
        process();

        REQUIRE(handle.type == InputCallbackType::MouseButton);

        clickMouse(MouseButton::Left);
        REQUIRE(count == 1u);

        callbacks.unregisterCallback(handle);
        process();
        clickMouse(MouseButton::Left);

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "unregister mouse move, scroll, and enter/exit callbacks", "[platform::inputCallbacks]")
    {
        uint32_t moveCount = 0u;
        uint32_t scrollCount = 0u;
        uint32_t enterExitCount = 0u;

        const auto move = callbacks.registerMouseMoveCallback([&moveCount](ServiceProvider&, MousePosition const&) { ++moveCount; return InputCallbackResult::Continue; });
        const auto scroll = callbacks.registerMouseScrollCallback([&scrollCount](ServiceProvider&, MousePosition const&, vec2) { ++scrollCount; return InputCallbackResult::Continue; });
        const auto enterExit = callbacks.registerMouseEnterExitCallback([&enterExitCount](ServiceProvider&, MousePosition const&) { ++enterExitCount; return InputCallbackResult::Continue; });
        process();

        REQUIRE(move.type == InputCallbackType::MouseMove);
        REQUIRE(scroll.type == InputCallbackType::MouseScroll);
        REQUIRE(enterExit.type == InputCallbackType::MouseEnterExit);

        moveMouse();
        scrollMouse();
        enterWindow();

        REQUIRE(moveCount == 1u);
        REQUIRE(scrollCount == 1u);
        REQUIRE(enterExitCount == 1u);

        callbacks.unregisterCallback(move);
        callbacks.unregisterCallback(scroll);
        callbacks.unregisterCallback(enterExit);
        process();

        moveMouse();
        scrollMouse();
        enterWindow();

        REQUIRE(moveCount == 1u);
        REQUIRE(scrollCount == 1u);
        REQUIRE(enterExitCount == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "unregister only removes the matching callback", "[platform::inputCallbacks]")
    {
        uint32_t removed = 0u;
        uint32_t kept = 0u;

        const auto handle = callbacks.registerKeyCallback(Key::A, countingKeyCallback(removed, InputCallbackResult::Continue));
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(kept, InputCallbackResult::Continue));
        process();

        callbacks.unregisterCallback(handle);
        process();
        pressKey(Key::A);

        REQUIRE(removed == 0u);
        REQUIRE(kept == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "register and unregister in the same batch", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        const auto handle = callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));

        callbacks.unregisterCallback(handle);
        process();
        pressKey(Key::A);

        REQUIRE(count == 0u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "unregister with a null handle is a no-op", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        callbacks.registerKeyCallback(Key::A, countingKeyCallback(count, InputCallbackResult::Continue));
        process();

        callbacks.unregisterCallback(InputCallbackHandle{});
        process();
        pressKey(Key::A);

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Re-entrancy
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "callback registered during dispatch fires next frame", "[platform::inputCallbacks]")
    {
        uint32_t lateCount = 0u;

        callbacks.registerKeyCallback(Key::A, [this, &lateCount](ServiceProvider&, Key, bool, bool, bool)
        {
            callbacks.registerKeyCallback(Key::A, countingKeyCallback(lateCount, InputCallbackResult::Continue));
            return InputCallbackResult::ContinueAndRemove;
        });
        process();

        pressKey(Key::A);
        REQUIRE(lateCount == 0u);

        process();
        pressKey(Key::A);
        REQUIRE(lateCount == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputCallbacksFixture, "callback unregistering itself during dispatch is safe", "[platform::inputCallbacks]")
    {
        uint32_t count = 0u;
        InputCallbackHandle self{};

        self = callbacks.registerKeyCallback(Key::A, [this, &self, &count](ServiceProvider&, Key, bool, bool, bool)
        {
            ++count;
            callbacks.unregisterCallback(self);
            return InputCallbackResult::Continue;
        });
        process();

        pressKey(Key::A);
        process();
        pressKey(Key::A);

        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE
}