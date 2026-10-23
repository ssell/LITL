# LITL Input — Design Reference

A high-level overview of the input system in `litl-platform` (`litl/platform/include/litl-platform/input`) and its GLFW backend in `litl-platform-glfw`: per-frame input snapshots, raw input callbacks, and action maps.

## Overview

The input system answers three questions, each building on the one before:

- **What happened to the devices this frame?** — an immutable per-frame snapshot of keys, mouse buttons, cursor position, and scroll (`Input`).
- **Who wants to react to it, and in what order?** — prioritized, consumable callbacks for raw events and actions (`InputCallbacks`).
- **What does the player *mean*?** — named actions such as `"forward"` bound to one or more physical sources, organized into a stack of action maps that control propagation and cursor mode (`InputActions` / `ActionMap`).

The pieces:

- **`Input`** — the public entry point and a DI service. Owns the frame snapshot, raw queries, cursor mode, and the `InputActions` instance. Talks to its backend through an `InputOps` function-pointer table, mirroring the renderer's split.
- **`InputState`** — the raw snapshot: keyboard, mouse, and gamepad bitsets plus mouse position, delta, scroll, and cursor mode.
- **`InputCallbacks`** — the callback registry shared by the engine and the user. Registration is deferred and thread-safe; dispatch is single-threaded on the main thread.
- **`InputSource`** — a device-neutral `{ type, code }` naming a single physical input (a key, a mouse button, the mouse delta, …).
- **`ActionMap`** — a named set of actions, each bound to up to 8 `InputSource`s, plus a cursor mode and a propagation policy.
- **`InputActions`** — owns all action maps and the active-map stack; resolves the snapshot into per-action `ActionMapState`s each frame.
- **`litl-platform-glfw`** — the backend: GLFW event callbacks write into an accumulator that `Input` drains once per frame.

The governing idea: **input is captured once per frame into an immutable snapshot on the main thread, before any system runs.** Systems read the snapshot (or the derived action states) in parallel without locks. Anything that changes input configuration — registering callbacks, pushing or popping maps — is deferred and applied at the next capture.

---

## Quick start

**Declare action maps** in the engine configuration. Note that this will be moved to the future editor.

```cpp
Configuration config{
    .platform = PlatformConfiguration{
        .input = InputConfiguration{
            .actionMaps = {
                ActionMapDescriptor{
                    .name = "ui",
                    .propagateActions = false,
                    .cursorMode = CursorMode::Normal,
                    .bindings = {
                        { "click", { InputSource::fromMouseButton(MouseButton::Left) } }
                    }
                },
                ActionMapDescriptor{
                    .name = "gameplay",
                    .propagateActions = false,
                    .cursorMode = CursorMode::Disabled,
                    .bindings = {
                        { "forward", { InputSource::fromKey(Key::W), InputSource::fromKey(Key::Up) } },
                        { "look",    { InputSource::fromMouseDelta() } },
                        { "escape",  { InputSource::fromKey(Key::Escape) } }
                    }
                }
            },
            .defaultActionMap = "ui"
        }
    }
};
```

**React to events** with callbacks, registered through the `ConfigureInputCallbacksFunc` passed to `Engine::setup`:

```cpp
void configureInputCallbacks(std::shared_ptr<InputCallbacks> callbacks)
{
    // Raw event
    callbacks->registerKeyCallback(Key::F1, [](ServiceProvider& services, Key, bool pressed, bool, bool)
    {
        if (pressed) { /* toggle something */ }
        return InputCallbackResult::Continue;
    }, InputCallbackPriority::Debug);

    // Action event: clicking in the "ui" map switches to "gameplay"
    callbacks->registerActionMapCallback("ui"_sid, "click"_sid, [](ServiceProvider& services, ActionMapState state)
    {
        if (state.released)
        {
            auto input = services.get<Input>();
            input->pushMap(input->getMap("gameplay"_sid));
        }

        return InputCallbackResult::Continue;
    });
}
```

**Or poll** from a system:

```cpp
void FreeFlySystem::setup(ServiceProvider& services)
{
    m_pInput = services.get<Input>();
    m_gameplay = m_pInput->getMap("gameplay"_sid);      // resolve the handle once
}

void FreeFlySystem::update(SystemData const& data, Entity entity, FreeFly& controller, Transform& transform)
{
    if (auto forward = m_pInput->getActionState(m_gameplay, "forward"_sid); forward && forward->held)
    {
        transform.translate(transform.forward() * data.deltaTime);
    }
}
```

See `samples/sponza` (action maps, polling, map switching) and `samples/boids` (raw mouse-button callbacks) for complete examples.

---

## Library layout and the backend seam

`litl-platform` holds the interface: `Input`, `InputState`, `InputCallbacks`, the action-map types, and the enums. It depends only on `litl-core` and knows nothing about GLFW.

`litl-platform-glfw` implements the backend. `Input` reaches it through an ops table, the same opaque-handle + function-pointer pattern used by `Renderer` / `litl-renderer-vulkan`:

```cpp
struct InputOps
{
    void (*destroy)(InputContext*);
    void (*setActiveWindow)(InputContext*, Window*);
    void (*poll)(InputContext*);
    InputState (*getAccumulatedState)(InputContext*, bool);
    void (*setCursorMode)(InputContext*, CursorMode);
};
```

`InputContext` is declared but never defined in `litl-platform`; each backend defines its own. The GLFW backend's context holds an **accumulator** `InputState` that GLFW's event callbacks write into (routed from the window via the `Platform*` stored as the GLFW window user pointer), and `getAccumulatedState` hands back a copy and clears its per-frame fields.

Translation from GLFW codes happens once, in `inputMappings.hpp`: dense `constexpr` tables map GLFW key and mouse-button codes to LITL's `Key` / `MouseButton`, with anything unmapped becoming `Unknown` and ignored. Static asserts pin each mapping.

The engine creates input through the platform during startup (`Platform::createInput(config)`), registers it as the `Input` singleton (non-owning; the backend owns the storage), and points it at the window with `setActiveWindow`.

---

## Frame lifecycle

```
Engine loop (main thread)
    Window::pollForEvents()              glfwPollEvents → GLFW callbacks → accumulator
    │
    World::run → onFrameStart
        Input::poll()
            ├─ backend poll              (gamepad polling will live here)
            ├─ snapshot = accumulator    (accumulator's edges are cleared)
            ├─ mouse delta               (suppressed on window entry / cursor-mode change)
            └─ InputActions::update      (apply push/pop, cursor mode, resolve action states)
        Input::triggerCallbacks()
            ├─ apply deferred callback registrations
            ├─ raw callbacks             (key, mouse button, enter/exit, move, scroll)
            └─ action callbacks
        │
        SystemGroup::Input, Update, …    systems read Input / action states in parallel
```

Everything in `onFrameStart` happens on the main thread before any system group runs. From then until the next frame's `onFrameStart`, the snapshot and action states are read-only, which is what makes them safe to read from parallel systems.

---

## Raw input — the snapshot

`InputState` is three device states plus the mouse position:

| Device | Per-frame | Persistent |
|--------|-----------|------------|
| Keyboard | `pressed`, `released` bitsets | `down` bitset |
| Mouse | `pressed`, `released`, `scroll`, `position.enteredWindow` / `exitedWindow`, `position.xyDelta` | `down`, `position.xy`, `position.inWindow`, `mode` |
| Gamepad | `pressed`, `released` | `down` |

The accumulator records **edges** (did a press/release event arrive?) separately from **state** (what was the last event?). That gives unambiguous semantics for every combination within a frame:

| `pressed` | `released` | `down` | Meaning |
|:-:|:-:|:-:|---|
| ✓ | | ✓ | Pressed this frame |
| | | ✓ | Held (pressed on an earlier frame) |
| | ✓ | | Released this frame |
| ✓ | ✓ | | Tapped: pressed and released within one frame |
| ✓ | ✓ | ✓ | Released and re-pressed within one frame |

The typed queries map onto this directly: `getKeyPressed`, `getKeyHeld` (which returns `down`, so it is also true on the press frame), and `getKeyReleased`, with matching `getMouseButton*` and `getGamepadButton*` variants. `getGenericInputSourceState(InputSource)` answers the same question for any source, returning an `InputSourceState { pressed, held, released, xyDelta }`.

Key-repeat events are ignored. When the window loses focus, GLFW synthesizes release events for anything held, so keys don't stick down across alt-tab.

### Mouse position and delta

`position.xy` is the cursor position as GLFW reports it (window coordinates). `position.xyDelta` is computed in `Input::poll` as the change since the previous snapshot, except on frames where the position is known to jump, when it is forced to zero:

- **The cursor entered the window**: the previous position is from wherever it left.
- **The cursor mode changed**: entering `Disabled` switches GLFW to a virtual, unbounded position, and leaving it restores the real one.

`scroll` accumulates every scroll event since the last poll; a single wheel notch is typically ±1 on `y`.

### Cursor modes

| `CursorMode` | Visible | Confined to window | Typical use |
|---|:-:|:-:|---|
| `Normal` | ✓ | | Menus, editor |
| `Hidden` | | | Custom-drawn cursors |
| `Captured` | ✓ | ✓ | RTS-style edge scrolling |
| `Disabled` | | ✓ (virtual, unbounded) | Mouse-look |

`Input::setCursorMode` applies a mode immediately on the main thread. In practice you rarely call it: when using action maps, the cursor mode follows the map at the top of the stack automatically.

---

## Input sources

`InputSource` is the device-neutral name for one physical input, used by action-map bindings and the generic state query:

```cpp
InputSource::fromKey(Key::W);
InputSource::fromMouseButton(MouseButton::Left);
InputSource::fromMouseDelta();
InputSource::fromMouseScroll();
InputSource::fromGamepadButton(GamepadButton::South);
InputSource::fromGamepadAxis(GamepadAxis::LeftX);      // not yet resolved — see "What's not yet here"
```

Buttons (keys, mouse buttons, gamepad buttons) produce `pressed` / `held` / `released`. The mouse delta and scroll produce `xyDelta`. `InputSourceState::merge` combines several sources by OR-ing the flags and summing the deltas; action maps use it to combine every binding of an action.

`Key` names keys by their **physical position on a US layout** (GLFW key codes are positional), so `Key::W` stays in the WASD position on AZERTY keyboards. `Key`, `MouseButton`, and `GamepadButton` are dense from `Unknown = 0` to `Count`, so they index bitsets and tables directly.

---

## Input callbacks

`InputCallbacks` is a single registry shared by the engine and the user. The engine owns it; the user receives it once through `ConfigureInputCallbacksFunc` at setup.

### Registration

| Method | Fires when |
|--------|------------|
| `registerKeyCallback(Key, …)` | That key was pressed or released this frame |
| `registerMouseButtonCallback(MouseButton, …)` | That button was pressed or released this frame |
| `registerMouseEnterExitCallback(…)` | The cursor entered or left the window |
| `registerMouseMoveCallback(…)` | The mouse delta is non-zero |
| `registerMouseScrollCallback(…)` | The scroll delta is non-zero |
| `registerActionMapCallback(mapId, bindingId, …)` | That action's state is `triggered` (see [Action state](#action-state)) |

**Callbacks fire on edges, not while held.** A key callback runs on the press frame and the release frame; to react every frame while something is held, poll instead.

Each `register*` returns an `InputCallbackHandle`, and `unregisterCallback(handle)` removes it. The handle carries everything needed to find the callback again (its type, the key/button slot, or the map and binding ids for action callbacks).

### Deferred, thread-safe registration

`register*` and `unregisterCallback` never touch the live callback lists. They append a command to a mutex-protected queue, and the queue is applied on the main thread at the start of the next `triggerCallbacks`, before anything is dispatched. Consequences:

- Registration and unregistration are safe from any thread, including parallel ECS systems.
- The lock is held only for a `push_back`, never while user code runs.
- A callback may register or unregister callbacks, **including itself**, while being dispatched; the change takes effect the next frame.
- A callback registered during dispatch does not receive the event currently being dispatched.

### Priority and propagation

Callbacks for the same event run in **descending priority**. Callbacks of equal priority run in registration order: insertion uses `std::upper_bound`, which places a new callback after existing ones of the same priority.

| `InputCallbackPriority` | Value | Intended for |
|---|---|---|
| `Debug` | 1000 | Debug toggles that must always see input |
| `Overlay` | 800 | Editor, consoles, modal dialogs with focus |
| `UI` | 600 | In-game UI that should consume clicks before gameplay |
| `Gameplay` | 400 | **Default.** Camera controllers, gameplay interactions |
| `Fallback` | 200 | Handles whatever nothing else consumed |

Levels are named by *purpose*, not by owner: engine and user callbacks share one registry and one scale. Any `uint32_t` value is valid (`InputCallbackPriority{ 450u }`), which is handy for slotting a callback between levels or pushing one above `Debug` while testing.

Each callback returns an `InputCallbackResult`:

| Result | Lower-priority callbacks for this event | This callback |
|---|---|---|
| `Continue` | Run | Kept |
| `Consume` | Skipped this frame | Kept |
| `ContinueAndRemove` | Run | Removed (one-shot) |
| `ConsumeAndRemove` | Skipped this frame | Removed (one-shot) |

A removed callback is marked dead immediately, so it cannot fire again later in the same dispatch, and it is erased from the list at the next deferred-command pass.

---

## Action maps

Action maps decouple *what the player pressed* from *what the player meant*, and let different contexts (gameplay, menus, an editor overlay) interpret the same keys differently.

### Defining maps

Maps are created once at startup from `InputConfiguration::actionMaps`; the set of maps is fixed after setup.

```cpp
struct ActionMapDescriptor
{
    std::string name;                     // hashed to the map's StringId
    bool propagateActions{ true };        // let unconsumed input fall through to maps below
    CursorMode cursorMode{ CursorMode::Normal };
    std::unordered_map<std::string, std::vector<InputSource>> bindings;    // action name → sources
};
```

Each action may bind up to 8 sources (`ActionMap::BindingsPerId`); extra sources are rejected with a warning. Actions and maps are identified by `StringId` (`"forward"_sid`). Resolve a map to an `ActionMapHandle` once with `Input::getMap` and reuse it.

### The stack

Active maps live on a stack. `pushMap(handle)` places a map on top; `popMap(handle)` removes the **top-most instance** of that map, wherever it is in the stack. Both are deferred commands applied at the start of the next `Input::poll`, and both are safe to call from any thread.

Each frame, `InputActions` walks the stack from the top down:

1. Each map visited is **live** this frame. Its actions read their bound sources from the snapshot.
2. A source that a higher map has already used is **consumed**: lower maps never see it, even when propagation is allowed. So a key bound in two live maps only drives the higher one.
3. If the map has `propagateActions = false`, the walk stops; every map below it is not live this frame.
4. A map that appears more than once in the stack is processed only at its highest position.

The stack does not need to be strictly LIFO. Two menus that don't take over input can be closed in either order, and the same map pushed twice (say, by two UI panels) stays active until it has been popped twice.

**Cursor mode follows the top of the stack.** When the top map changes, its `cursorMode` is applied; an empty stack means `Normal`. In Sponza, pushing `gameplay` (`Disabled`) captures the mouse for looking around, and popping it on Escape releases the cursor back to `ui` (`Normal`).

### Action state

Each action resolves to an `ActionMapState` every frame, for **every** map, whether or not it is live:

```cpp
struct ActionMapState
{
    bool live;        // the action's map was reached during this frame's stack walk
    bool held;        // any bound source is down (and the map is live)
    bool pressed;     // the action went from not-held to held, or was tapped
    bool released;    // the action went from held to not-held, or was tapped
    bool triggered;   // pressed || released || value != 0
    vec2 value;       // summed delta of bound delta sources (mouse delta, scroll)
};
```

All of an action's sources are **merged** before edges are computed, and edges are computed against the action's own previous state rather than taken from any single source:

```
held      = live && any(source.held)
pressed   = (held && !prevHeld) || (live && any(source.pressed) && !prevHeld)
released  = (prevHeld && !held)  || (live && any(source.released) && !held)
value     = live ? sum(source.xyDelta) : 0
```

This gives the behaviour you would expect from an action rather than from a key:

- **Multiple bindings behave as one.** Holding Up and then pressing W does not re-press "forward"; releasing W while Up is still held does not release it.
- **Taps survive.** A press and release within one frame reports both `pressed` and `released`.
- **Leaving the stack cancels.** When a map stops being live (popped, or a blocking map pushed above it), every action that was held reports `released` for exactly one frame, then goes quiet, even though the key may still be physically down. A "move forward" action can't keep running underneath a menu.
- **Returning to a map while a key is held synthesizes a press.** Close the menu while holding W and "forward" reports `pressed` immediately, so movement resumes without re-pressing the key.
- **A higher map taking over a key releases it below.** If a propagating map is pushed and claims a key a lower map's action was holding, the lower action is released.

Poll with `Input::getActionState(handle, bindingId)`, which returns `std::nullopt` for unknown maps or bindings. Action callbacks receive the same `ActionMapState`, and are dispatched whenever `triggered` is set, including for the synthetic releases above. `held` alone never triggers a callback, matching raw callbacks.

---

## Threading

| Operation | Thread | Notes |
|-----------|--------|-------|
| GLFW event callbacks | Main | Write only to the backend accumulator |
| `Input::poll`, `triggerCallbacks` | Main | Called by the engine in `onFrameStart` |
| Raw queries, `getGenericInputSourceState`, `getActionState`, `getMap` | Any | Read-only between frame starts |
| `register*` / `unregisterCallback` | Any | Deferred, mutex-protected queue |
| `pushMap` / `popMap` | Any | Deferred, mutex-protected queue |
| `Input::setCursorMode` | Main | Calls into GLFW directly |
| Callback bodies | Main | Run during `onFrameStart`, before any system group |

The safety argument is the same as for `SceneView`: the snapshot and action states are only written in `onFrameStart`, on the main thread, while no systems are running. During system execution they are immutable.

---

## Conventions and invariants

### One snapshot per frame

Everything a frame observes about input comes from the snapshot taken in that frame's `onFrameStart`. Events that arrive later (for example, while the renderer waits on a minimized window) land in the accumulator and appear in the next snapshot; nothing is lost and nothing changes mid-frame.

### Edges versus state

`pressed` / `released` are edges and can both be true in one frame; `down` / `held` is state. Don't infer a tap from `pressed && !held`; check `pressed && released`.

### Deferred configuration

Callback registration and map push/pop take effect at the next frame start, never immediately. This is what makes them safe from systems and from inside callbacks.

### Names are hashed

Maps and actions are identified by `StringId`. A misspelt name is not a compile error; it shows up as an invalid handle or a `std::nullopt` state. Resolve handles once (for example, in a system's `setup`) and check them.

---

## What's not yet here

Gaps worth knowing about, for context on the current shape:

- **Gamepads.** The enums, bitsets, and `InputSource` factories exist, but the GLFW backend's `poll` does not yet call `glfwGetGamepadState`, and `getGenericInputSourceState` does not resolve `GamepadAxis` (it logs a warning).
- **Composite and typed actions.** There is no WASD → `vec2` composite; `ActionType` (`Button` / `Axis1D` / `Axis2D`) is declared but not used. Movement is expressed as separate button actions, as in Sponza.
- **Window coordinates versus framebuffer pixels.** Cursor positions are GLFW window coordinates. With display scaling these differ from the framebuffer size used by cameras, so screen-to-world picking can be offset on high-DPI displays.
- **Rebinding and serialization.** Maps are defined in code at startup and are immutable afterwards; there is no runtime rebinding or loading from files.
- **Modifiers, chords, and interactions.** No Shift+X bindings, and no hold / double-tap / long-press interactions.
- **Text input.** No character or IME events; `Key` is for physical keys only.
- **`InputCallbacks` as a service.** The registry currently reaches user code only through `ConfigureInputCallbacksFunc`. Injecting it through DI, so systems can register at runtime, is planned.
- **Single window.** Input is bound to the one active window.

Each is a deliberate deferral; none is locked out by the current structure.

---

## Useful files to read

| File | What lives here |
|------|-----------------|
| `litl/platform/include/litl-platform/input/input.hpp` | `Input` public surface, `InputOps` |
| `litl/platform/src/input/input.cpp` | Snapshot capture, mouse-delta suppression, raw callback dispatch |
| `litl/platform/include/litl-platform/input/inputState.hpp` | `InputState` and the per-device states |
| `litl/platform/include/litl-platform/input/inputEnums.hpp` | `Key`, `MouseButton`, `GamepadButton`, `GamepadAxis`, `CursorMode` |
| `litl/platform/include/litl-platform/input/inputSource.hpp` | `InputSource`, `InputSourceState`, `merge` |
| `litl/platform/include/litl-platform/input/inputCallbacks.hpp` | Callback signatures, priorities, results, handles |
| `litl/platform/src/input/inputCallbacks.cpp` | Deferred command queue, priority insertion, dispatch |
| `litl/platform/include/litl-platform/input/actionMapDescriptor.hpp` | Action map configuration |
| `litl/platform/src/input/actionMap.cpp` | Source merging and action edge rules |
| `litl/platform/src/input/inputActions.cpp` | Map stack, propagation, consumption, cursor mode |
| `litl/platform/src/input/processedInputTracker.cpp` | Per-frame record of consumed sources |
| `litl/platform-glfw/src/input.cpp` | GLFW accumulator and cursor-mode application |
| `litl/platform-glfw/src/include/inputMappings.hpp` | GLFW → LITL key and button tables |
| `litl/engine/src/engineCallbacks.cpp` | Where `poll` and `triggerCallbacks` are wired into `onFrameStart` |
| `samples/sponza/src/freeFlyController.cpp` | Action-map polling for a free-fly camera |
| `samples/boids/src/main.cpp` | Raw mouse-button callbacks with screen-to-world picking |

For how `onFrameStart` fits into the frame, see [`ecs.md`](./ecs.md).