#ifndef LITL_ENGINE_INPUT_ACTIONS_H__
#define LITL_ENGINE_INPUT_ACTIONS_H__

#include <memory>
#include <optional>

#include "litl-core/stringId.hpp"
#include "litl-core/handles.hpp"
#include "litl-engine/input/actionMap.hpp"

namespace litl
{
    class Engine;
    class ServiceProvider;

    struct ActionMapHandleTag {};
    using ActionMapHandle = Handle<ActionMapHandleTag>;

    /// <summary>
    /// Service built on top of Input that allows for more sophisticated input operations.
    /// It does not replace raw Input queries or callbacks, but allows for features such as:
    /// 
    /// * Unified Input: Multiple input sources tied to the same event. For example, both W and UpArrow keys tied to "MoveForward".
    /// * Automatic Cursor Mode Update: Cursor mode is updated to match whichever ActionMap is the active map.
    /// * Down-Stack Propagation: ActionMaps can allow, or prevent, cascading action propagation. For example, menus may want to prevent actions from reaching underlying gameplay action maps.
    /// 
    /// InputActions manages active/propagating actions via an internal stack. The map at the top of the stack is the current active map
    /// and controls setting such as cursor mode. Each map can allow, or disallow, unconsumed actions from propagating down the stack.
    /// To enable an action map, the caller must invoke pushMap. This will enqueue a command to place the map at the top of the stack.
    /// When the map is no longer in use, the caller is then responsible for calling popMap.
    /// 
    /// Note that the push/pop does not have to be in-sync. For example if the game allows for multiple menus which do not take over
    /// input, the menus can be closed in whichever order that the player wishes.
    /// 
    /// Additionally, the same action map may be present in the stack at the same time. If multiple GUI elements all call pushMap("Menu")
    /// then "Menu" will be in the stack more than once. This is fine as consumed actions, even if propagation is allowed, do not fall down the stack.
    /// So even with multiple instances of "Menu" on the stack, the actual "Menu" action map will only trigger once per action.
    /// 
    /// Finally, InputActions is thread-safe and can be safely used from a parallel ECS system. Pushing and popping maps are both deferred commands
    /// that are not processed until a safe sync point. Creating and retrieving maps employ locks to safeguard the underlying containers. Because of 
    /// the associated cost of locks it is recommended to request a map once and reuse it, for example in an ECS system ::setup method.
    /// </summary>
    class InputActions final
    {
    public:

        InputActions();
        ~InputActions();

        InputActions(InputActions const&) = delete;
        InputActions operator=(InputActions const&) = delete;

        /// <summary>
        /// Sets up InputActions with DI services.
        /// All action maps are created from those defined in the injected Configuration's InputConfiguration.
        /// </summary>
        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;

        /// <summary>
        /// Retrieves the action map with the given id.
        /// It is recommended to retrieve the map once and then store the handle as this is a locking call.
        /// </summary>
        [[nodiscard]] ActionMapHandle getMap(StringId id) noexcept;

        /// <summary>
        /// Enqueues a command to push the specified map to the top of the stack.
        /// This map will become the active map and its settings (cursor mode, action propagation, etc.) will be used.
        /// </summary>
        void pushMap(ActionMapHandle handle) noexcept;

        /// <summary>
        /// Enqueues a command to remove the specified map from the stack.
        /// </summary>
        void popMap(ActionMapHandle handle) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif