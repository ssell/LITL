#ifndef LITL_ENGINE_INPUT_ACTION_MAP_H__
#define LITL_ENGINE_INPUT_ACTION_MAP_H__

#include <array>
#include <cstdint>
#include <memory>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-engine/input/actionMapDescriptor.hpp"

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

    class ActionMap
    {
    public:

        struct DefaultIds
        {
            static constexpr StringId UI = "UI"_sid;
            static constexpr StringId Gameplay = "Gameplay"_sid;
        };

        [[nodiscard]] bool create(Authority<InputActions> auth, ActionMapDescriptor const& descriptor) noexcept;

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

        struct ActionBinding
        {
            std::string name;
            StringId id;
            InputSource source{};
        };

        static constexpr uint32_t BindingsPerId = 8u;
        ActionMapDescriptor m_descriptor{};
        StringId m_id{};
        StringIdMap<std::array<ActionBinding, BindingsPerId>> m_actionBindings;
    };
}

#endif