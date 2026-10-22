#ifndef LITL_PLATFORM_INPUT_ACTION_MAP_H__
#define LITL_PLATFORM_INPUT_ACTION_MAP_H__

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types/vec2.hpp"
#include "litl-platform/input/actionMapDescriptor.hpp"
#include "litl-platform/input/actionMapState.hpp"
#include "litl-platform/input/processedInputTracker.hpp"

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

    struct ActionMapBinding
    {
        StringId actionMapId{};
        StringId bindingId{};
    };

    class ActionMap
    {
    public:

        /// <summary>
        /// Attempts to create the action map using the provided descriptor.
        /// </summary>
        [[nodiscard]] bool create(Authority<InputActions> auth, ActionMapDescriptor const& descriptor) noexcept;

        /// <summary>
        /// Invoked for all action maps each update.
        /// </summary>
        void onUpdate(Authority<InputActions> auth);

        /// <summary>
        /// Updates the input state of the action map and invokes any callbacks. Returns true if propagation of input should continue.
        /// This is only invoked on action maps visited during the stack traversal.
        /// </summary>
        [[nodiscard]] bool updateInput(Authority<InputActions> auth, Input const& input, ProcessedInputTracker& inputTracker, std::vector<ActionMapBinding>& triggeredBindings) noexcept;

        /// <summary>
        /// Invoked for all action maps following the input update to sync their action states.
        /// </summary>
        void onPostUpdate(Authority<InputActions> auth) noexcept;

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

        [[nodiscard]] std::optional<ActionMapState> getBindingState(StringId bindingId) const noexcept;

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
            ActionMapState actionState{};
            InputSourceState inputState{};
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