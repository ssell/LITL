

#include "litl-engine/input/actionMap.hpp"
#include "litl-engine/input/inputActions.hpp"
#include "litl-platform/input.hpp"

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

            for (auto& binding : grouping.second)
            {
                bind(grouping.first, binding);
            }

            m_actionBindingKeys.push_back(StringId(grouping.first));
        }

        return true;
    }

    bool ActionMap::update(Authority<InputActions> auth, Input const& input, ProcessedInputTracker& inputTracker) noexcept
    {
        // Clear all input source states
        for (auto& actionBindings : m_actionBindings)
        {
            actionBindings.state = {};
        }
    
        // Update input source states with the latest input
        for (const auto key : m_actionBindingKeys)
        {
            auto& actionBindings = getActionBindings(key);

            for (auto& binding : actionBindings.bindings)
            {
                // Go down the list of each binding until we find an input that could have triggered it.
                // For example, if both Key:W and Key::Up are bound to "MoveForward" then continue until one of those has input.
                const auto state = input.getGenericInputSourceState(binding.source);

                if (state.isActive())
                {
                    actionBindings.state = state;                       // Record the first active input state into the overall ActionBindings for the key
                    inputTracker.markProcessed(binding.source);         // Mark the individual input source as processed

                    // ... todo ... invoke callbacks ...

                    break;                                              // Exit this action bindings once we have found a single active input source
                }
            }
        }

        return m_descriptor.propagateActions;
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

        return false;
    }
    
    void ActionMap::setActiveMap(Authority<InputActions> auth) noexcept
    {
        // ... todo ...
    }

    void ActionMap::unsetActiveMap(Authority<InputActions> auth) noexcept
    {
        // ... todo ...
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

    std::optional<InputSourceState> ActionMap::getBindingState(StringId bindingId) const noexcept
    {
        auto find = m_actionBindingsMap.find(bindingId);

        if (find == m_actionBindingsMap.end())
        {
            return std::nullopt;
        }

        return m_actionBindings[find->second].state;
    }

    ActionMap::ActionBindings& ActionMap::getActionBindings(StringId id) noexcept
    {
        return m_actionBindings[m_actionBindingsMap[id]];
    }
}