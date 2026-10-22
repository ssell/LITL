#include <thread>
#include <vector>

#include "litl-platform/tests-input.hpp"

namespace litl::tests
{
    namespace
    {
        constexpr StringId GameplayId = "gameplay"_sid;
        constexpr StringId UiId = "ui"_sid;

        constexpr StringId Forward = "forward"_sid;
        constexpr StringId Right = "right"_sid;
        constexpr StringId Look = "look"_sid;
        constexpr StringId Next = "next"_sid;
        constexpr StringId Confirm = "confirm"_sid;

        ActionMapDescriptor gameplayMap()
        {
            return ActionMapDescriptor{
                .name = "gameplay",
                .propagateActions = false,
                .cursorMode = CursorMode::Disabled,
                .bindings = {
                    { "forward", { InputSource::fromKey(Key::W), InputSource::fromKey(Key::Up) } },
                    { "right",   { InputSource::fromKey(Key::D), InputSource::fromKey(Key::Right) } },
                    { "look",    { InputSource::fromMouseDelta() } }
                }
            };
        }

        /// <summary>
        /// UI shares Key::Right with gameplay ("next" vs "right") to exercise consumption.
        /// </summary>
        ActionMapDescriptor uiMap(bool propagate)
        {
            return ActionMapDescriptor{
                .name = "ui",
                .propagateActions = propagate,
                .cursorMode = CursorMode::Normal,
                .bindings = {
                    { "next",    { InputSource::fromKey(Key::Right) } },
                    { "confirm", { InputSource::fromKey(Key::Enter) } }
                }
            };
        }

        struct InputActionsFixture : InputFixture
        {
            ActionMapHandle gameplay{};
            ActionMapHandle ui{};

            /// <summary>
            /// Starts with "gameplay" as the default map, then runs two frames: the first applies the default
            /// push (and its cursor mode), the second absorbs the resulting cursor-mode change.
            /// </summary>
            void startActions(bool uiPropagates = false)
            {
                start(InputConfiguration{
                    .actionMaps = { gameplayMap(), uiMap(uiPropagates) },
                    .defaultActionMap = "gameplay"
                    });

                gameplay = input.getMap(GameplayId);
                ui = input.getMap(UiId);

                REQUIRE(gameplay.isValid());
                REQUIRE(ui.isValid());

                frame();
                frame();
            }
        };
    }

    // -------------------------------------------------------------------------------------
    // Setup and lookup
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputActionsFixture, "default map is pushed on the first poll and sets the cursor mode", "[platform::inputActions]")
    {
        start(InputConfiguration{ .actionMaps = { gameplayMap(), uiMap(false) }, .defaultActionMap = "gameplay" });

        REQUIRE(backend.setCursorModeCalls == 0u);

        frame();
        REQUIRE(backend.cursorMode == CursorMode::Disabled);
        REQUIRE(backend.setCursorModeCalls == 1u);

        backend.pressKey(Key::W);
        frame();
        REQUIRE(action(GameplayId, Forward).pressed);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "unknown maps and bindings return nothing", "[platform::inputActions]")
    {
        startActions();

        REQUIRE_FALSE(input.getMap("missing"_sid).isValid());
        REQUIRE_FALSE(input.getActionState(input.getMap("missing"_sid), Forward).has_value());
        REQUIRE_FALSE(input.getActionState(gameplay, "missing"_sid).has_value());
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Button actions
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputActionsFixture, "button action press, hold, and release", "[platform::inputActions]")
    {
        startActions();

        backend.pressKey(Key::W);
        frame();
        auto state = action(GameplayId, Forward);
        REQUIRE(state.pressed);
        REQUIRE(state.held);
        REQUIRE(state.triggered);
        REQUIRE_FALSE(state.released);

        frame();
        state = action(GameplayId, Forward);
        REQUIRE(state.held);
        REQUIRE_FALSE(state.pressed);
        REQUIRE_FALSE(state.triggered);         // held alone does not trigger

        backend.releaseKey(Key::W);
        frame();
        state = action(GameplayId, Forward);
        REQUIRE(state.released);
        REQUIRE(state.triggered);
        REQUIRE_FALSE(state.held);

        frame();
        state = action(GameplayId, Forward);
        REQUIRE_FALSE(state.pressed);
        REQUIRE_FALSE(state.held);
        REQUIRE_FALSE(state.released);
        REQUIRE_FALSE(state.triggered);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "single-frame tap is pressed and released but not held", "[platform::inputActions]")
    {
        startActions();

        backend.tapKey(Key::W);
        frame();

        const auto state = action(GameplayId, Forward);
        REQUIRE(state.pressed);
        REQUIRE(state.released);
        REQUIRE(state.triggered);
        REQUIRE_FALSE(state.held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "releasing one binding while another is held keeps the action held", "[platform::inputActions]")
    {
        startActions();

        backend.pressKey(Key::Up);
        frame();
        backend.pressKey(Key::W);
        frame();

        backend.releaseKey(Key::W);
        frame();
        auto state = action(GameplayId, Forward);
        REQUIRE(state.held);
        REQUIRE_FALSE(state.released);
        REQUIRE_FALSE(state.pressed);

        backend.releaseKey(Key::Up);
        frame();
        state = action(GameplayId, Forward);
        REQUIRE(state.released);
        REQUIRE_FALSE(state.held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "a second binding pressed while the first is held does not re-press", "[platform::inputActions]")
    {
        startActions();

        backend.pressKey(Key::Up);
        frame();

        backend.pressKey(Key::W);
        frame();

        const auto state = action(GameplayId, Forward);
        REQUIRE(state.held);
        REQUIRE_FALSE(state.pressed);
        REQUIRE_FALSE(state.triggered);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "both bindings pressed in one frame produce a single callback", "[platform::inputActions]")
    {
        startActions();

        uint32_t count = 0u;
        callbacks.registerActionMapCallback(GameplayId, Forward, [&count](ServiceProvider&, ActionMapState) { ++count; return InputCallbackResult::Continue; });

        backend.pressKey(Key::W);
        backend.pressKey(Key::Up);
        frame();

        REQUIRE(count == 1u);
        REQUIRE(action(GameplayId, Forward).pressed);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Axis actions
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputActionsFixture, "mouse delta action reports the delta for one frame", "[platform::inputActions]")
    {
        startActions();

        backend.moveMouseTo(100.0f, 100.0f);
        frame();

        backend.moveMouseTo(103.0f, 98.0f);
        frame();
        auto state = action(GameplayId, Look);
        REQUIRE(state.value.x() == 3.0f);
        REQUIRE(state.value.y() == -2.0f);
        REQUIRE(state.triggered);

        frame();
        state = action(GameplayId, Look);
        REQUIRE(state.value.isZeroed());
        REQUIRE_FALSE(state.triggered);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Stack behaviour
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputActionsFixture, "maps not on the stack do not respond", "[platform::inputActions]")
    {
        startActions();

        backend.pressKey(Key::Enter);
        frame();

        const auto state = action(UiId, Confirm);
        REQUIRE_FALSE(state.pressed);
        REQUIRE_FALSE(state.held);
        REQUIRE_FALSE(state.triggered);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "pushing a blocking map releases held actions below it exactly once", "[platform::inputActions]")
    {
        startActions(false);

        backend.pressKey(Key::W);
        frame();
        REQUIRE(action(GameplayId, Forward).held);

        input.pushMap(ui);
        frame();
        auto state = action(GameplayId, Forward);
        REQUIRE(state.released);
        REQUIRE(state.triggered);
        REQUIRE_FALSE(state.held);

        frame();                                // W is still physically down
        state = action(GameplayId, Forward);
        REQUIRE_FALSE(state.released);
        REQUIRE_FALSE(state.held);
        REQUIRE_FALSE(state.triggered);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "returning to a map while its key is held synthesizes a press", "[platform::inputActions]")
    {
        startActions(false);

        backend.pressKey(Key::W);
        frame();
        input.pushMap(ui);
        frame();

        input.popMap(ui);
        frame();

        // Intended: movement resumes immediately when the menu closes.
        const auto state = action(GameplayId, Forward);
        REQUIRE(state.pressed);
        REQUIRE(state.held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "a propagating top map consumes shared keys and passes others down", "[platform::inputActions]")
    {
        startActions(true);

        input.pushMap(ui);
        frame();

        backend.pressKey(Key::Right);
        frame();
        REQUIRE(action(UiId, Next).pressed);
        REQUIRE_FALSE(action(GameplayId, Right).held);

        backend.pressKey(Key::W);
        frame();
        REQUIRE(action(GameplayId, Forward).pressed);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "a propagating map taking over a held key releases it below", "[platform::inputActions]")
    {
        startActions(true);

        backend.pressKey(Key::Right);
        frame();
        REQUIRE(action(GameplayId, Right).held);

        input.pushMap(ui);
        frame();

        const auto lower = action(GameplayId, Right);
        REQUIRE(lower.released);
        REQUIRE_FALSE(lower.held);

        const auto upper = action(UiId, Next);
        REQUIRE(upper.pressed);
        REQUIRE(upper.held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "a map pushed twice stays active until popped twice", "[platform::inputActions]")
    {
        startActions(false);

        input.pushMap(ui);
        input.pushMap(ui);
        frame();

        input.popMap(ui);
        frame();

        backend.pressKey(Key::Enter);
        frame();
        REQUIRE(action(UiId, Confirm).pressed);

        input.popMap(ui);
        backend.pressKey(Key::W);
        frame();
        REQUIRE(action(GameplayId, Forward).pressed);
        REQUIRE_FALSE(action(UiId, Confirm).held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "popping a map below the top removes it without changing the top", "[platform::inputActions]")
    {
        startActions(true);

        input.pushMap(ui);
        frame();
        REQUIRE(backend.cursorMode == CursorMode::Normal);

        input.popMap(gameplay);
        frame();

        backend.pressKey(Key::W);
        backend.pressKey(Key::Enter);
        frame();

        REQUIRE_FALSE(action(GameplayId, Forward).pressed);
        REQUIRE(action(UiId, Confirm).pressed);
        REQUIRE(backend.cursorMode == CursorMode::Normal);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "push and pop in the same frame leave held actions untouched", "[platform::inputActions]")
    {
        startActions(false);

        backend.pressKey(Key::W);
        frame();

        input.pushMap(ui);
        input.popMap(ui);
        frame();

        const auto state = action(GameplayId, Forward);
        REQUIRE(state.held);
        REQUIRE_FALSE(state.pressed);
        REQUIRE_FALSE(state.released);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "cursor mode follows the top of the stack", "[platform::inputActions]")
    {
        startActions(false);
        REQUIRE(backend.cursorMode == CursorMode::Disabled);
        REQUIRE(backend.setCursorModeCalls == 1u);

        input.pushMap(ui);
        frame();
        REQUIRE(backend.cursorMode == CursorMode::Normal);
        REQUIRE(backend.setCursorModeCalls == 2u);

        input.pushMap(ui);                      // top unchanged
        frame();
        REQUIRE(backend.setCursorModeCalls == 2u);

        input.popMap(ui);                       // top is still ui
        frame();
        REQUIRE(backend.setCursorModeCalls == 2u);

        input.popMap(ui);
        frame();
        REQUIRE(backend.cursorMode == CursorMode::Disabled);
        REQUIRE(backend.setCursorModeCalls == 3u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "push requested from another thread is applied on the next poll", "[platform::inputActions]")
    {
        startActions(false);

        std::thread worker([this]() { input.pushMap(ui); });
        worker.join();

        REQUIRE(backend.cursorMode == CursorMode::Disabled);
        frame();
        REQUIRE(backend.cursorMode == CursorMode::Normal);
    }
    LITL_END_TEST_CASE

        // -------------------------------------------------------------------------------------
        // Action callbacks
        // -------------------------------------------------------------------------------------

    LITL_TEST_CASE_METHOD(InputActionsFixture, "action callbacks fire on press and release but not while held", "[platform::inputActions]")
    {
        startActions();

        std::vector<ActionMapState> log;
        callbacks.registerActionMapCallback(GameplayId, Forward, [&log](ServiceProvider&, ActionMapState state) { log.push_back(state); return InputCallbackResult::Continue; });

        backend.pressKey(Key::W);
        frame();
        frame();
        frame();
        backend.releaseKey(Key::W);
        frame();

        REQUIRE(log.size() == 2u);
        REQUIRE(log[0].pressed);
        REQUIRE(log[1].released);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "action callbacks receive the cancellation release", "[platform::inputActions]")
    {
        startActions(false);

        std::vector<ActionMapState> log;
        callbacks.registerActionMapCallback(GameplayId, Forward, [&log](ServiceProvider&, ActionMapState state) { log.push_back(state); return InputCallbackResult::Continue; });

        backend.pressKey(Key::W);
        frame();

        input.pushMap(ui);
        frame();

        REQUIRE(log.size() == 2u);
        REQUIRE(log[1].released);
        REQUIRE_FALSE(log[1].held);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "unregistered action callbacks stop firing", "[platform::inputActions]")
    {
        startActions();

        uint32_t count = 0u;
        const auto handle = callbacks.registerActionMapCallback(GameplayId, Forward, [&count](ServiceProvider&, ActionMapState) { ++count; return InputCallbackResult::Continue; });

        REQUIRE(handle.type == InputCallbackType::ActionMap);
        REQUIRE(handle.actionMapId == GameplayId);
        REQUIRE(handle.bindingId == Forward);

        backend.pressKey(Key::W);
        frame();
        REQUIRE(count == 1u);

        callbacks.unregisterCallback(handle);
        backend.releaseKey(Key::W);
        frame();
        REQUIRE(count == 1u);
    }
    LITL_END_TEST_CASE

    LITL_TEST_CASE_METHOD(InputActionsFixture, "action callback priority and consume", "[platform::inputActions]")
    {
        startActions();

        std::vector<uint32_t> order;

        callbacks.registerActionMapCallback(GameplayId, Forward, [&order](ServiceProvider&, ActionMapState) { order.push_back(1u); return InputCallbackResult::Continue; }, InputCallbackPriority::Gameplay);
        callbacks.registerActionMapCallback(GameplayId, Forward, [&order](ServiceProvider&, ActionMapState) { order.push_back(2u); return InputCallbackResult::Consume; }, InputCallbackPriority::UI);

        backend.pressKey(Key::W);
        frame();
        REQUIRE(order == std::vector<uint32_t>{ 2u });

        backend.releaseKey(Key::W);
        frame();
        REQUIRE(order == std::vector<uint32_t>{ 2u, 2u });     // consume does not disable the callback
    }
    LITL_END_TEST_CASE
}