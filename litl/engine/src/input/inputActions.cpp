#include <mutex>
#include <ranges>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/input/inputActions.hpp"
#include "litl-engine/engine.hpp"
#include "litl-platform/input.hpp"

namespace litl
{
    namespace
    {
        enum class InputActionCommandType : uint8_t
        {
            Unknown = 0u,
            Push    = 1u,
            Pop     = 2u
        };

        struct InputActionCommand
        {
            ActionMapHandle handle{};
            InputActionCommandType type{ InputActionCommandType::Unknown };
        };
    }

    struct InputActions::Impl
    {
        std::shared_ptr<Input> input{ nullptr };
        HandlePool<ActionMap, ActionMapHandleTag> actionMapPool{};
        StringIdMap<ActionMapHandle> actionMapLookup;
        std::vector<ActionMapHandle> actionMapStack;
        std::vector<InputActionCommand> actionMapCommands;
        std::mutex commandsMutex{};

        void setup(ServiceProvider& services) noexcept
        {
            input = services.get<Input>();
            auto config = services.get<Configuration>();

            LITL_FATAL_ASSERT_MSG((input != nullptr), "Failed to inject Input into InputActions.");
            LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into InputActions.");

            for (auto& descriptor : config->input.actionMaps)
            {
                if (!descriptor.isValid())
                {
                    logWarning("InputConfiguration provided with invalid ActionMapDescriptor '", descriptor.name, "'");
                    continue;
                }

                ActionMap actionMap{};

                if (!actionMap.create({}, descriptor))
                {
                    logError("Failed to create ActionMap");
                    continue;
                }

                if (!actionMapPool.create(actionMap).isValid())
                {
                    logError("Failed to create ActionMapHandle for ActionMap '", descriptor.name, "'");
                }
            }
        }

        [[nodiscard]] ActionMapHandle getMap(StringId id) noexcept
        {
            // Since we enforce when action maps can be created (during setup only) we do not need to lock the lookup in a critical section.
            auto find = actionMapLookup.find(id);

            if (find == actionMapLookup.end())
            {
                return {};
            }

            return find->second;
        }

        void pushMap(ActionMapHandle handle) noexcept
        {
            if (!handle.isValid())
            {
                return;
            }

            std::scoped_lock lock{ commandsMutex };

            actionMapCommands.push_back(InputActionCommand{
                .handle = handle,
                .type = InputActionCommandType::Push
            });
        }

        void popMap(ActionMapHandle handle) noexcept
        {
            if (!handle.isValid())
            {
                return;
            }

            std::scoped_lock lock{ commandsMutex };

            actionMapCommands.push_back(InputActionCommand{
                .handle = handle,
                .type = InputActionCommandType::Pop
            });
        }

        void processCommand(InputActionCommand const& command) noexcept
        {
            switch (command.type)
            {
            case InputActionCommandType::Push:
                processCommandPush(command);
                break;

            case InputActionCommandType::Pop:
                processCommandPop(command);
                break;

            default:
                break;
            }
        }

        void setTopActive(bool isActive) noexcept
        {
            if (actionMapStack.empty())
            {
                return;
            }

            const ActionMapHandle topHandle = actionMapStack.back();
            ActionMap* topActionMap = actionMapPool.get(topHandle);

            if (topActionMap != nullptr)
            {
                if (isActive)
                {
                    topActionMap->setActiveMap({});
                }
                else
                {
                    topActionMap->unsetActiveMap({});
                }
            }
        }

        void processCommandPush(InputActionCommand const& command) noexcept
        {
            setTopActive(false);
            actionMapStack.push_back(command.handle);
            setTopActive(true);
        }

        void processCommandPop(InputActionCommand const& command) noexcept
        {
            if (actionMapStack.empty())
            {
                return;
            }

            if (actionMapStack.back() == command.handle)
            {
                // Requesting to pop the top.
                setTopActive(false);
                actionMapStack.pop_back();
                setTopActive(true);
            }
            else
            {
                // Otherwise we must descend the stack until we find the top-most instance of the specified action map and then remove it.
                for (auto riter = actionMapStack.rbegin(); riter != actionMapStack.rend(); ++riter)
                {
                    if (*riter == command.handle)
                    {
                        actionMapStack.erase(std::next(riter).base());
                        break;
                    }
                }
            }
        }
    };

    InputActions::InputActions()
        : m_pImpl(std::make_unique<Impl>())
    {
        m_pImpl->actionMapStack.reserve(32u);
    }

    InputActions::~InputActions()
    {

    }

    void InputActions::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pImpl->setup(services);
    }

    ActionMapHandle InputActions::getMap(StringId id) noexcept
    {
        return m_pImpl->getMap(id);
    }

    void InputActions::pushMap(ActionMapHandle handle) noexcept
    {
        m_pImpl->pushMap(handle);
    }

    void InputActions::popMap(ActionMapHandle handle) noexcept
    {
        m_pImpl->popMap(handle);
    }

    // ... todo need: 
    // ... propagate input to action maps so they can trigger callbacks
}