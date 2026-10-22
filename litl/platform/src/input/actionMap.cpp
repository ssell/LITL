#include "litl-platform/input/actionMap.hpp"
#include "litl-platform/input/inputActions.hpp"
#include "litl-platform/input/input.hpp"

namespace litl
{
    bool ActionMap::create(Authority<InputActions> auth, ActionMapDescriptor const& descriptor) noexcept
    {
        if (!descriptor.isValid())
        {
            return false;
        }

        m_descriptor = descriptor;
        m_id = StringId(m_descriptor.name);

        m_actionBindingKeys.reserve(m_descriptor.bindings.size());
        m_actionBindings.reserve(m_descriptor.bindings.size());

        for (auto& grouping : m_descriptor.bindings)
        {
            if (grouping.second.empty())
            {
                continue;
            }

            bool anySuccess = false;

            for (auto& binding : grouping.second)
            {
                anySuccess |= bind(grouping.first, binding);
            }

            if (anySuccess)
            {
                m_actionBindingKeys.push_back(StringId(grouping.first));
            }
        }

        return true;
    }

    void ActionMap::onUpdate(Authority<InputActions> auth)
    {
        // Simply set all bindings .live to false. They will be set to true if visited in updateInput.
        // All other states will be updated in onPostUpdate once input has been processed.
        for (auto& bindings : m_actionBindings)
        {
            bindings.actionState.live = false;
        }
    }

    bool ActionMap::updateInput(Authority<InputActions> auth, Input const& input, ProcessedInputTracker& inputTracker) noexcept
    {
        // Clear all input source states
        for (auto& actionBindings : m_actionBindings)
        {
            actionBindings.inputState = {};
        }
    
        // Update input source states with the latest input
        for (const auto key : m_actionBindingKeys)
        {
            auto& actionBindings = getActionBindings(key);
            actionBindings.actionState.live = true;

            for (auto& binding : actionBindings.bindings)
            {
                // Go down the list of each binding and merge all input source states.
                const auto state = input.getGenericInputSourceState(binding.source);

                if (state.isActive() && !inputTracker.isProcessed(binding.source))
                {
                    // Mark the individual input source as processed
                    inputTracker.markProcessed(binding.source);
                    actionBindings.inputState = InputSourceState::merge(actionBindings.inputState, state);
                }
            }
        }

        return m_descriptor.propagateActions;
    }

    void ActionMap::onPostUpdate(Authority<InputActions> auth, std::vector<ActionMapBinding>& triggeredBindings) noexcept
    {
        for (auto& bindings : m_actionBindings)
        {
            const ActionMapState prevState = bindings.actionState;
            ActionMapState& currState = bindings.actionState;

            currState.held      = currState.live && bindings.inputState.held;
            currState.pressed   = (currState.held && !prevState.held) || (currState.live && bindings.inputState.pressed && !prevState.held);        // If map just became live, then synthesize held if map became active while the key was down. Otherwise compare to last frame.
            currState.released  = (prevState.held && !currState.held) || (currState.live && bindings.inputState.released && !currState.held);       // If map just became live, then synthesize released if map became active same frame it was released. Otherwise compare to last frame.
            currState.value     = (currState.live ? bindings.inputState.xyDelta : vec2{});
            currState.triggered = currState.pressed || currState.released || !currState.value.isZeroed();                                           // Note: we do not mark triggered for .held. This matches the behavior of our raw input callbacks.

            if (currState.triggered)
            {
                triggeredBindings.push_back(ActionMapBinding{
                    .actionMapId = m_id,
                    .bindingId = bindings.id
                });
            }
        }
    }

    bool ActionMap::bind(std::string_view name, InputSource source) noexcept
    {
        if (name.empty() || (source.type == InputSourceType::Unknown))
        {
            return false;
        }

        const auto id = StringId(name);
        auto find = m_actionBindingsMap.find(id);

        if (find == m_actionBindingsMap.end())
        {
            m_actionBindingsMap[id] = m_actionBindings.size();
            m_actionBindings.push_back(ActionBindings{
                .name = std::string(name),
                .id = StringId(name)
            });
        }

        auto& actionBindings = getActionBindings(id);

        for (uint32_t i = 0u; i < BindingsPerId; ++i)
        {
            if (actionBindings.bindings[i].source.type == InputSourceType::Unknown)
            {
                actionBindings.bindings[i] = ActionBinding{
                    .name = std::string(name),
                    .id = id,
                    .source = source
                };

                return true;
            }
        }

        logWarning("Failed to bind action '", name, "' to map '", getName(), "'. Each binding may have up to ", BindingsPerId, " individual sources.");

        return false;
    }

    ActionMapDescriptor const& ActionMap::getDescriptor() const noexcept
    {
        return m_descriptor;
    }

    StringId ActionMap::getId() const noexcept
    {
        return m_id;
    }

    std::string_view ActionMap::getName() const noexcept
    {
        return m_descriptor.name;
    }

    std::optional<ActionMapState> ActionMap::getBindingState(StringId bindingId) const noexcept
    {
        auto find = m_actionBindingsMap.find(bindingId);

        if (find == m_actionBindingsMap.end())
        {
            return std::nullopt;
        }

        return m_actionBindings[find->second].actionState;
    }

    ActionMap::ActionBindings& ActionMap::getActionBindings(StringId id) noexcept
    {
        return m_actionBindings[m_actionBindingsMap[id]];
    }
}