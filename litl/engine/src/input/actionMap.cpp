#include <array>
#include "litl-engine/input/actionMap.hpp"

namespace litl
{
    struct ActionMap::Impl
    {
        static constexpr uint32_t BindingsPerId = 8u;
        StringIdMap<std::array<InputSource, BindingsPerId>> actionBindings;

        [[nodiscard]] bool bind(StringId name, InputSource source) noexcept
        {
            auto& bindings = actionBindings[name];

            for (uint32_t i = 0u; i < BindingsPerId; ++i)
            {
                if (bindings[i].type == InputSourceType::Unknown)
                {
                    bindings[i] = source;
                    return true;
                }
            }

            return false;
        }

        void unbind(StringId name, InputSource source) noexcept
        {
            auto& bindings = actionBindings[name];

            for (uint32_t i = 0u; i < BindingsPerId; ++i)
            {
                if (bindings[i] == source)
                {
                    bindings[i] = {};
                }
            }
        }

        void unbindAll(StringId name) noexcept
        {
            actionBindings.erase(name);
        }
    };

    ActionMap::ActionMap()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    ActionMap::~ActionMap()
    {

    }

    bool ActionMap::bind(StringId name, InputSource source) noexcept
    {
        return m_pImpl->bind(name, source);
    }
    
    void ActionMap::unbind(StringId name, InputSource source) noexcept
    {
        m_pImpl->unbind(name, source);
    }

    void ActionMap::unbindAll(StringId name) noexcept
    {
        m_pImpl->unbindAll();
    }
}