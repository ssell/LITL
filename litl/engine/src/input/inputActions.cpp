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
            StringId id{};
            InputActionCommandType type{ InputActionCommandType::Unknown };
        };
    }

    struct InputActions::Impl
    {
        std::mutex actionMapsMutex{};
        std::mutex commandsMutex{};

        std::shared_ptr<Input> input{ nullptr };
        std::stack<StringId> mapStack;
        StringIdMap<std::unique_ptr<ActionMap>> actionMaps;         // Use unique_ptr so references via getMap are stable even if the underlying map is resized.
        std::vector<InputActionCommand> commands;

        void setup(ServiceProvider& services) noexcept
        {
            input = services.get<Input>();
            LITL_FATAL_ASSERT_MSG((input != nullptr), "Failed to inject Input into InputActions.");
        }

        [[nodiscard]] bool createMap(ActionMapDescriptor const& descriptor) noexcept
        {
            if (!descriptor.isValid())
            {
                return false;
            }

            const auto id = StringId(descriptor.name);

            {
                std::scoped_lock lock(actionMapsMutex);

                if (actionMaps.find(id) != actionMaps.end())
                {
                    return false;
                }

                actionMaps[id] = std::make_unique<ActionMap>(descriptor);

                return true;
            }
        }

        [[nodiscard]] ActionMap* getMap(StringId id) noexcept
        {
            std::scoped_lock lock(actionMapsMutex);

            auto find = actionMaps.find(id);

            if (find == actionMaps.end())
            {
                return nullptr;
            }

            return find->second.get();
        }

        void pushMap(StringId id) noexcept
        {
            std::scoped_lock lock{ commandsMutex };

            commands.push_back(InputActionCommand{
                .id = id,
                .type = InputActionCommandType::Push
            });
        }

        void popMap(StringId id) noexcept
        {
            std::scoped_lock lock{ commandsMutex };

            commands.push_back(InputActionCommand{
                .id = id,
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

            auto top = actionMaps.find(mapStack.top());

            if (top != actionMaps.end())
            {
                if (isActive)
                {
                    top->second->setActiveMap({});
                }
                else
                {
                    top->second->unsetActiveMap({});
                }
            }
        }

        void processCommandPush(InputActionCommand const& command) noexcept
        {
            setTopActive(false);
            mapStack.push(command.id);
            setTopActive(true);
        }

        void processCommandPop(InputActionCommand const& command) noexcept
        {
            setTopActive(false);
            mapStack.pop();
            setTopActive(true);
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

    void InputActions::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pImpl->setup(services);
    }

    bool InputActions::createMap(ActionMapDescriptor const& descriptor) noexcept
    {
        return m_pImpl->createMap(descriptor);
    }

    ActionMap* InputActions::getMap(StringId id) noexcept
    {
        return m_pImpl->getMap(id);
    }

    void InputActions::pushMap(StringId id) noexcept
    {
        m_pImpl->pushMap(id);
    }

    void InputActions::popMap(StringId id) noexcept
    {
        m_pImpl->popMap(id);
    }

    // ... todo need: 
    // ... process deferred commands
    // ... propagate input to action maps so they can trigger callbacks
}