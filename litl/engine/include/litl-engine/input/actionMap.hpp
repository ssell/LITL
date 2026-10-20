#ifndef LITL_ENGINE_INPUT_ACTION_MAP_H__
#define LITL_ENGINE_INPUT_ACTION_MAP_H__

#include <cstdint>
#include <memory>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-engine/input/inputSource.hpp"

namespace litl
{
    class InputActions;

    enum class ActionType : uint8_t
    {
        Unknown = 0u,
        Button = 1u,
        Axis1D = 2u,
        Axis2D = 3u
    };

    struct ActionState
    {
        bool pressed{ false };
        bool released{ false };
        bool down{ false };
        vec2 value{};
    };

    struct ActionMapDescriptor
    {
        /// <summary>
        /// The unique name of this map used both for display and to hash and serve as the key.
        /// </summary>
        std::string name;

        /// <summary>
        /// Should any actions not consumed by this map be propagated down the stack?
        /// </summary>
        bool propagateActions{ true };

        /// <summary>
        /// The cursor mode applied when this map is the active map at the top of the stack.
        /// </summary>
        CursorMode cursorMode{ CursorMode::Normal };
    };

    class ActionMap
    {
    public:

        struct DefaultIds
        {
            static constexpr StringId UI = "UI"_sid;
            static constexpr StringId Gameplay = "Gameplay"_sid;
        };

        ActionMap(ActionMapDescriptor const& descriptor);
        ~ActionMap();

        ActionMap(ActionMap const&) = delete;
        ActionMap& operator=(ActionMap const&) = delete;

        // ---------------------------------------------------------------------------------
        // Bind / Unbind
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Adds a binding with the given name for the specified input source.
        /// For example: Bind "Move Forward" with "Key W"
        /// </summary>
        bool bind(std::string_view name, InputSource source) noexcept;

        /// <summary>
        /// Unbind the specific input source and name combination.
        /// For example: Unbind "Key W" from "Move Forward"
        /// </summary>
        void unbind(std::string_view name, InputSource source) noexcept;

        /// <summary>
        /// Unbinds all bindings with the specified name.
        /// For example: Unbind "Move Forward" which unbinds "Key W", "Key Up Arrow", etc.
        /// </summary>
        void unbindAll(std::string_view name) noexcept;

        // ---------------------------------------------------------------------------------
        // Set / Unset Active
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Sets the map as the active map. This updates the cursor mode, etc.
        /// </summary>
        void setActiveMap(Authority<InputActions> auth) noexcept;

        /// <summary>
        /// Unsets the map as the active map. This causes the map to mimic cancellation of any active actions.
        /// For example, any relevant keys being held down are simulated to be released. This prevents unwanted
        /// actions such as "Move Forward" from continuing indefinitely while a menu (separate action map) is open on top.
        /// </summary>
        /// <param name="auth"></param>
        void unsetActiveMap(Authority<InputActions> auth) noexcept;

        // ---------------------------------------------------------------------------------
        // Getters
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Returns the descriptor used to create this action map.
        /// </summary>
        [[nodiscard]] ActionMapDescriptor const& getDescriptor() const noexcept;

        /// <summary>
        /// Returns the id assigned to the action map.
        /// </summary>
        [[nodiscard]] StringId getId() const noexcept;

        /// <summary>
        /// Returns the name of the action map.
        /// </summary>
        [[nodiscard]] std::string_view getName() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl{ nullptr };
    };
}

#endif