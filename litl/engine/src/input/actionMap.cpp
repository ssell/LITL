

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
        for (const auto key : m_actionBindingKeys)
        {
            auto& bindings = m_actionBindings[key];

            for (const auto& binding : bindings)
            {
                // Go down the list of each binding until we find an input that could have triggered it.
                // For example, if both Key:W and Key::Up are bound to "MoveForward" then continue until one of those has input.
                const auto state = input.getGenericInputSourceState(binding.source);

                if (state.isActive())
                {
                    inputTracker.markProcessed(binding.source);

                    // ... todo ... act on it ...
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
        auto& bindings = m_actionBindings[id];

        for (uint32_t i = 0u; i < BindingsPerId; ++i)
        {
            if (bindings[i].source.type == InputSourceType::Unknown)
            {
                bindings[i] = ActionBinding{
                    .name = std::string(name),
                    .id = id,
                    .source = source
                };

                return true;
            }
        }

        return false;
    }

    void ActionMap::unbind(std::string_view name, InputSource source) noexcept
    {
        if (name.empty() || (source.type == InputSourceType::Unknown))
        {
            return;
        }

        auto& bindings = m_actionBindings[name];

        for (uint32_t i = 0u; i < BindingsPerId; ++i)
        {
            if (bindings[i].source == source)
            {
                bindings[i] = {};
            }
        }
    }

    void ActionMap::unbindAll(std::string_view name) noexcept
    {
        m_actionBindings.erase(name);
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
}