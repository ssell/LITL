#ifndef LITL_ENGINE_INPUT_ACTION_MAP_H__
#define LITL_ENGINE_INPUT_ACTION_MAP_H__

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-engine/input/actionMapDescriptor.hpp"
#include "litl-engine/input/processedInputTracker.hpp"

namespace litl
{
    class InputActions;
    class Input;

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

        /// <summary>
        /// Attempts to create the action map using the provided descriptor.
        /// </summary>
        [[nodiscard]] bool create(Authority<InputActions> auth, ActionMapDescriptor const& descriptor) noexcept;

        /// <summary>
        /// Updates the input state of the action map and invokes any callbacks.
        /// Returns true if propagation of input should continue.
        /// </summary>
        [[nodiscard]] bool update(Authority<InputActions> auth, Input const& input, ProcessedInputTracker& inputTracker) noexcept;

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

        [[nodiscard]] std::optional<InputSourceState> getBindingState(StringId bindingId) const noexcept;

    private:

        static constexpr uint32_t BindingsPerId = 8u;

        struct ActionBinding
        {
            std::string name;
            StringId id;
            InputSource source{};
        };

        struct ActionBindings
        {
            std::string name;
            StringId id;
            InputSourceState state{};
            std::array<ActionBinding, BindingsPerId> bindings;
        };

        bool bind(std::string_view name, InputSource source) noexcept;
        [[nodiscard]] ActionBindings& getActionBindings(StringId id) noexcept;

        ActionMapDescriptor m_descriptor{};
        StringId m_id{};
        StringIdMap<size_t> m_actionBindingsMap;
        std::vector<StringId> m_actionBindingKeys;
        std::vector<ActionBindings> m_actionBindings;
    };
}

#endif