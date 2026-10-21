

#include "litl-engine/input/actionMap.hpp"
#include "litl-engine/input/inputActions.hpp"

namespace litl
{
    namespace
    {
    }

    bool ActionMap::create(Authority<InputActions> auth, ActionMapDescriptor const& descriptor) noexcept
    {
        if (!descriptor.isValid())
        {
            return false;
        }

        m_descriptor = descriptor;
        m_id = StringId(m_descriptor.name);

        return true;
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