#include <mutex>
#include <stack>
#include "litl-engine/input/inputActions.hpp"

namespace litl
{
    namespace
    {
        enum class InputActionCommandType : uint8_t
        {
            Unknown = 0u,
            Push = 1u,
            Pop  = 2u
        };

        struct InputActionCommand
        {
            StringId id{};
            InputActionCommandType type{ InputActionCommandType::Unknown };
        };
    }

    struct InputActions::Impl
    {
        std::stack<StringId> mapStack;
        StringIdMap<ActionMap> actionMaps;

        std::mutex commandsMutex{};
        std::vector<InputActionCommand> commands;

        [[nodiscard]] ActionMap& getMap(StringId id) noexcept
        {
            return actionMaps[id];
        }

        [[nodiscard]] bool pushMap(StringId id) noexcept
        {
            if (!actionMaps.contains(id))
            {
                return false;
            }

            std::scoped_lock lock{ commandsMutex };

            commands.push_back(InputActionCommand{
                .id = id,
                .type = InputActionCommandType::Push
            });

            return true;
        }

        void popMap() noexcept
        {
            std::scoped_lock lock{ commandsMutex };

            commands.push_back(InputActionCommand{
                .type = InputActionCommandType::Pop
            });
        }

        void processCommand(InputActionCommand const& command) noexcept
        {
            switch (command.type)
            {
            case InputActionCommandType::Push:
                setTopActive(false);
                mapStack.push(command.id);
                setTopActive(true);
                break;

            case InputActionCommandType::Pop:
                setTopActive(false);
                mapStack.pop();
                setTopActive(true);
                break;

            default:
                break;
            }
        }

        void setTopActive(bool isActive) noexcept
        {
            if (mapStack.empty())
            {
                return;
            }

            auto top = actionMaps.find(mapStack.top());

            if (top != actionMaps.end())
            {
                if (isActive)
                {
                    top->second.setActiveMap({});
                }
                else
                {
                    top->second.unsetActiveMap({});
                }
            }
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

    // ... todo need: 
    // ... process deferred commands
    // ... create action maps via InputActions
    // ... propagate input to action maps so they can trigger callbacks
    // ... add getter for action maps
}