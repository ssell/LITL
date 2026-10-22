#include "litl-platform/tests-input.hpp"
#include "litl-platform/input/processedInputTracker.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Raw key / mouse state
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputFixture, "key press, hold, and release", "[platform::input]")
    {
        start();

        backend.pressKey(Key::W);
        frame();
        REQUIRE(input.getKeyPressed(Key::W));
        REQUIRE(input.getKeyHeld(Key::W));
        REQUIRE_FALSE(input.getKeyReleased(Key::W));

        frame();
        REQUIRE_FALSE(input.getKeyPressed(Key::W));
        REQUIRE(input.getKeyHeld(Key::W));
        REQUIRE_FALSE(input.getKeyReleased(Key::W));

        backend.releaseKey(Key::W);
        frame();
        REQUIRE_FALSE(input.getKeyPressed(Key::W));
        REQUIRE_FALSE(input.getKeyHeld(Key::W));
        REQUIRE(input.getKeyReleased(Key::W));

        frame();
        REQUIRE_FALSE(input.getKeyPressed(Key::W));
        REQUIRE_FALSE(input.getKeyHeld(Key::W));
        REQUIRE_FALSE(input.getKeyReleased(Key::W));
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "single-frame tap is pressed and released but not held", "[platform::input]")
    {
        start();

        backend.tapKey(Key::Space);
        frame();

        REQUIRE(input.getKeyPressed(Key::Space));
        REQUIRE(input.getKeyReleased(Key::Space));
        REQUIRE_FALSE(input.getKeyHeld(Key::Space));
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "mouse button press and release", "[platform::input]")
    {
        start();

        backend.pressMouseButton(MouseButton::Left);
        frame();
        REQUIRE(input.getMouseButtonPressed(MouseButton::Left));
        REQUIRE(input.getMouseButtonHeld(MouseButton::Left));

        backend.releaseMouseButton(MouseButton::Left);
        frame();
        REQUIRE(input.getMouseButtonReleased(MouseButton::Left));
        REQUIRE_FALSE(input.getMouseButtonHeld(MouseButton::Left));
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Mouse delta and scroll
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputFixture, "mouse delta is the change in position between polls", "[platform::input]")
    {
        start();

        backend.enterWindowAt(100.0f, 100.0f);
        frame();

        backend.moveMouseTo(110.0f, 95.0f);
        frame();
        REQUIRE(input.getMousePosition().xyDelta.x() == 10.0f);
        REQUIRE(input.getMousePosition().xyDelta.y() == -5.0f);

        frame();
        REQUIRE(input.getMousePosition().xyDelta.isZeroed());
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "entering the window suppresses the delta for that poll", "[platform::input]")
    {
        start();

        backend.moveMouseTo(10.0f, 10.0f);
        frame();

        backend.enterWindowAt(500.0f, 500.0f);
        frame();
        REQUIRE(input.getMouseEntered());
        REQUIRE(input.getMousePosition().xyDelta.isZeroed());

        backend.moveMouseTo(505.0f, 500.0f);
        frame();
        REQUIRE(input.getMousePosition().xyDelta.x() == 5.0f);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "cursor mode changes suppress the delta for that poll", "[platform::input]")
    {
        start();

        backend.enterWindowAt(100.0f, 100.0f);
        frame();

        // Entering Disabled jumps to a virtual cursor position.
        input.setCursorMode(CursorMode::Disabled);
        backend.moveMouseTo(4000.0f, 3000.0f);
        frame();
        REQUIRE(input.getMousePosition().xyDelta.isZeroed());

        backend.moveMouseTo(4010.0f, 3000.0f);
        frame();
        REQUIRE(input.getMousePosition().xyDelta.x() == 10.0f);

        // Leaving Disabled restores the real cursor position, which can also jump.
        input.setCursorMode(CursorMode::Normal);
        backend.moveMouseTo(100.0f, 100.0f);
        frame();
        REQUIRE(input.getMousePosition().xyDelta.isZeroed());
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "scroll is reported for a single poll", "[platform::input]")
    {
        start();

        backend.scroll(0.0f, 2.0f);
        frame();
        REQUIRE(input.getMouseScroll().y() == 2.0f);

        frame();
        REQUIRE(input.getMouseScroll().isZeroed());
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Generic input sources
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputFixture, "generic input source state matches the typed queries", "[platform::input]")
    {
        start();

        backend.enterWindowAt(0.0f, 0.0f);
        frame();

        backend.pressKey(Key::W);
        backend.pressMouseButton(MouseButton::Right);
        backend.moveMouseTo(3.0f, 4.0f);
        backend.scroll(0.0f, -1.0f);
        frame();

        const auto key = input.getGenericInputSourceState(InputSource::fromKey(Key::W));
        REQUIRE(key.pressed);
        REQUIRE(key.held);
        REQUIRE_FALSE(key.released);

        const auto button = input.getGenericInputSourceState(InputSource::fromMouseButton(MouseButton::Right));
        REQUIRE(button.pressed);
        REQUIRE(button.held);

        const auto delta = input.getGenericInputSourceState(InputSource::fromMouseDelta());
        REQUIRE(delta.xyDelta.x() == 3.0f);
        REQUIRE(delta.xyDelta.y() == 4.0f);

        const auto scroll = input.getGenericInputSourceState(InputSource::fromMouseScroll());
        REQUIRE(scroll.xyDelta.y() == -1.0f);

        const auto idle = input.getGenericInputSourceState(InputSource::fromKey(Key::Q));
        REQUIRE_FALSE(idle.isActive());
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE("input source state merge", "[platform::input]")
    {
        const InputSourceState a{ .pressed = true, .held = true, .released = false, .xyDelta = vec2{ 1.0f, 2.0f } };
        const InputSourceState b{ .pressed = false, .held = false, .released = true, .xyDelta = vec2{ 3.0f, -1.0f } };

        const auto merged = InputSourceState::merge(a, b);

        REQUIRE(merged.pressed);
        REQUIRE(merged.held);
        REQUIRE(merged.released);
        REQUIRE(merged.xyDelta.x() == 4.0f);
        REQUIRE(merged.xyDelta.y() == 1.0f);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Raw callbacks dispatched through Input
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputFixture, "raw key callbacks fire on edges only", "[platform::input]")
    {
        start();

        uint32_t count = 0u;
        callbacks.registerKeyCallback(Key::W, [&count](ServiceProvider&, Key, bool, bool, bool) { ++count; return InputCallbackResult::Continue; });

        backend.pressKey(Key::W);
        frame();
        REQUIRE(count == 1u);

        frame();                // held, no edge
        REQUIRE(count == 1u);

        backend.releaseKey(Key::W);
        frame();
        REQUIRE(count == 2u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputFixture, "raw mouse move callback receives the delta", "[platform::input]")
    {
        start();

        vec2 received{};
        callbacks.registerMouseMoveCallback([&received](ServiceProvider&, MousePosition const& mouse) { received = mouse.xyDelta; return InputCallbackResult::Continue; });

        backend.enterWindowAt(50.0f, 50.0f);
        frame();

        backend.moveMouseTo(52.0f, 47.0f);
        frame();

        REQUIRE(received.x() == 2.0f);
        REQUIRE(received.y() == -3.0f);
    }
    LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // ProcessedInputTracker
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("processed input tracker", "[platform::input]")
    {
        ProcessedInputTracker tracker{};

        const auto key = InputSource::fromKey(Key::W);
        const auto delta = InputSource::fromMouseDelta();
        const auto scroll = InputSource::fromMouseScroll();

        SECTION("starts with nothing processed")
        {
            REQUIRE_FALSE(tracker.isProcessed(key));
            REQUIRE_FALSE(tracker.isProcessed(delta));
            REQUIRE_FALSE(tracker.isProcessed(scroll));
        }

        SECTION("unknown sources are always treated as processed")
        {
            REQUIRE(tracker.isProcessed(InputSource{}));
        }

        SECTION("marking mouse delta does not mark scroll")
        {
            tracker.markProcessed(delta);
            REQUIRE(tracker.isProcessed(delta));
            REQUIRE_FALSE(tracker.isProcessed(scroll));
        }

        SECTION("sources with equal codes but different types do not alias")
        {
            const auto sameCodeKey = InputSource::fromKey(static_cast<Key>(static_cast<uint32_t>(MouseButton::Left)));
            const auto button = InputSource::fromMouseButton(MouseButton::Left);
            const auto gamepadButton = InputSource::fromGamepadButton(static_cast<GamepadButton>(0u));
            const auto gamepadAxis = InputSource::fromGamepadAxis(static_cast<GamepadAxis>(0u));

            tracker.markProcessed(button);
            tracker.markProcessed(gamepadButton);

            REQUIRE(tracker.isProcessed(button));
            REQUIRE(tracker.isProcessed(gamepadButton));
            REQUIRE_FALSE(tracker.isProcessed(sameCodeKey));
            REQUIRE_FALSE(tracker.isProcessed(gamepadAxis));
        }

        SECTION("last gamepad axis is in range")
        {
            const auto lastAxis = InputSource::fromGamepadAxis(static_cast<GamepadAxis>(static_cast<uint32_t>(GamepadAxis::Count) - 1u));
            tracker.markProcessed(lastAxis);
            REQUIRE(tracker.isProcessed(lastAxis));
        }

        SECTION("reset clears everything")
        {
            tracker.markProcessed(key);
            tracker.markProcessed(delta);
            tracker.reset();

            REQUIRE_FALSE(tracker.isProcessed(key));
            REQUIRE_FALSE(tracker.isProcessed(delta));
        }
    }
    LITL_END_TEST_CASE
}