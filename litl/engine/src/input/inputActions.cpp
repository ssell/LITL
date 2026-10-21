#include <mutex>
#include <stack>

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
        HandlePool<ActionMap, ActionMapHandleTag> actionMapPool{};
        StringIdMap<ActionMapHandle> actionMapLookup;
        std::vector<ActionMapHandle> actionMapStack;

        std::shared_ptr<Input> input{ nullptr };
        std::stack<StringId> mapStack;
        std::vector<InputActionCommand> commands;

        void setup(ServiceProvider& services) noexcept
        {
            input = services.get<Input>();
            LITL_FATAL_ASSERT_MSG((input != nullptr), "Failed to inject Input into InputActions.");
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

            commands.push_back(InputActionCommand{
                .handle = handle,
                .type = InputActionCommandType::Push
            });
        }

        void popMap(ActionMapHandle handle) noexcept
        {
            commands.push_back(InputActionCommand{
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
            if (mapStack.empty())
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
            setTopActive(false);
            actionMapStack.pop_back();
            setTopActive(true);
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
    // ... process deferred commands
    // ... propagate input to action maps so they can trigger callbacks
}