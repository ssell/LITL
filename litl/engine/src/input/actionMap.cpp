#include <array>

#include "litl-engine/input/actionMap.hpp"
#include "litl-engine/input/inputActions.hpp"

namespace litl
{
    namespace
    {
        struct ActionBinding
        {
            std::string name;
            StringId id;
            InputSource source{};
        };
    }

    struct ActionMap::Impl
    {
        static constexpr uint32_t BindingsPerId = 8u;

        ActionMapDescriptor descriptor{};
        StringId id{};
        StringIdMap<std::array<ActionBinding, BindingsPerId>> actionBindings;

        [[nodiscard]] bool bind(std::string_view name, InputSource source) noexcept
        {
            if (name.empty() || (source.type == InputSourceType::Unknown))
            {
                return false;
            }

            const auto id = StringId(name);
            auto& bindings = actionBindings[id];

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

        void unbind(std::string_view name, InputSource source) noexcept
        {
            if (name.empty() || (source.type == InputSourceType::Unknown))
            {
                return;
            }

            auto& bindings = actionBindings[name];

            for (uint32_t i = 0u; i < BindingsPerId; ++i)
            {
                if (bindings[i].source == source)
                {
                    bindings[i] = {};
                }
            }
        }

        void unbindAll(std::string_view name) noexcept
        {
            actionBindings.erase(name);
        }
    };

    ActionMap::ActionMap(ActionMapDescriptor const& descriptor)
        : m_pImpl(std::make_unique<Impl>())
    {
        m_pImpl->descriptor = descriptor;
        m_pImpl->id = StringId(m_pImpl->descriptor.name);
    }

    ActionMap::~ActionMap()
    {

    }

    bool ActionMap::bind(std::string_view name, InputSource source) noexcept
    {
        return m_pImpl->bind(name, source);
    }
    
    void ActionMap::unbind(std::string_view name, InputSource source) noexcept
    {
        m_pImpl->unbind(name, source);
    }

    void ActionMap::unbindAll(std::string_view name) noexcept
    {
        m_pImpl->unbindAll(name);
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
        return m_pImpl->descriptor;
    }

    StringId ActionMap::getId() const noexcept
    {
        return m_pImpl->id;
    }

    std::string_view ActionMap::getName() const noexcept
    {
        return m_pImpl->descriptor.name;
    }
}