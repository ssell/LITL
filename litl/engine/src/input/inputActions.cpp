#include <stack>
#include "litl-engine/input/inputActions.hpp"

namespace litl
{
    struct InputActions::Impl
    {
        std::stack<StringId> mapStack;
        StringIdMap<ActionMap> actionMaps;

        [[nodiscard]] ActionMap& getMap(StringId id) noexcept
        {
            return actionMaps[id];
        }

        [[nodiscard]] bool pushMap(StringId id) noexcept
        {
            if (id == mapStack.top())
            {
                // Already the active map.
                return true;
            }

            auto find = actionMaps.find(id);

            if (find == actionMaps.end())
            {
                // This map does not exist.
                return false;
            }

            if (!mapStack.empty())
            {
                // Gracefully cancel any non-zero actions in the active/top map.
                // For example if we are transitioning to a menu while the player is moving then we want to send mock released=true,down=false input events to cancel the movement.

                // ... todo ...
            }

            mapStack.push(id);

            return true;
        }

        void popMap() noexcept
        {
            if (actionMaps.empty())
            {
                return;
            }

            // cancel active map ...
            // ... todo ...

            mapStack.pop();
        }
    };

    InputActions::InputActions()
        : m_pImpl(std::make_unique<Impl>())
    {
        m_pImpl->actionMaps.reserve(16u);
    }

    InputActions::~InputActions()
    {

    }

    ActionMap& InputActions::getMap(StringId id) noexcept
    {
        return m_pImpl->getMap(id);
    }

    bool InputActions::pushMap(StringId id) noexcept
    {
        return m_pImpl->pushMap(id);
    }

    void InputActions::popMap() noexcept
    {
        m_pImpl->popMap();
    }
}