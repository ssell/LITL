#include <mutex>
#include <ranges>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/input/inputActions.hpp"
#include "litl-engine/engine.hpp"
#include "litl-platform/input.hpp"
#include "litl-engine/input/actionMap.hpp"

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
        std::vector<ActionMapHandle> allActionMapHandles;               // This is also tracked by actionMapPool but eliminates the need for us to call getAllHandles each frame since maps are static once they exist.
        std::vector<ActionMapHandle> actionMapStack;
        std::vector<InputActionCommand> actionMapCommands;
        std::vector<bool> actionMapProcessed;
        std::mutex commandsMutex{};
        ProcessedInputTracker processedInputTracker{};

        // ---------------------------------------------------------------------------------
        // Setup
        // ---------------------------------------------------------------------------------

        void setup(ServiceProvider& services) noexcept
        {
            input = services.get<Input>();
            auto config = services.get<Configuration>();

            LITL_FATAL_ASSERT_MSG((input != nullptr), "Failed to inject Input into InputActions.");
            LITL_FATAL_ASSERT_MSG((config != nullptr), "Failed to inject Configuration into InputActions.");

            allActionMapHandles.reserve(config->input.actionMaps.size());

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

                const auto handle = actionMapPool.create(actionMap);

                if (!handle.isValid())
                {
                    logError("Failed to create ActionMapHandle for ActionMap '", descriptor.name, "'");
                }

                allActionMapHandles.push_back(handle);
            }

            actionMapProcessed.resize(allActionMapHandles.size(), false);
        }

        // ---------------------------------------------------------------------------------
        // Update
        // ---------------------------------------------------------------------------------

        void update() noexcept
        {
            processCommands();
            updateActionMapsWithInput();
        }

        void processCommands() noexcept
        {
            if (!actionMapCommands.empty())
            {
                for (auto& command : actionMapCommands)
                {
                    processCommand(command);
                }

                actionMapCommands.clear();
            }
        }

        void updateActionMapsWithInput() noexcept
        {
            if (allActionMapHandles.empty())
            {
                return;
            }

            // The same map may be present in the stack multiple times. Track which have already been processed to avoid invoking multiple times.
            actionMapProcessed.assign(actionMapProcessed.size(), false);
            uint32_t remainingUnprocessed = static_cast<uint32_t>(actionMapProcessed.size());
            processedInputTracker.reset();

            for (auto actionMapIter = actionMapStack.rbegin(); (actionMapIter != actionMapStack.rend()) && (remainingUnprocessed != 0u); ++actionMapIter)
            {
                const auto handle = *actionMapIter;

                if (actionMapProcessed[handle.index])
                {
                    // This map was already processed.
                    continue;
                }

                actionMapProcessed[handle.index] = true;
                remainingUnprocessed--;

                auto* actionMap = actionMapPool.get(*actionMapIter);

                if (actionMap != nullptr)
                {
                    if (!actionMap->update({}, *input, processedInputTracker))
                    {
                        // This action map does not allow propagation of input down the stack.
                        break;
                    }
                }
            }
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

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

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

        [[nodiscard]] std::optional<InputSourceState> getInputState(ActionMapHandle handle, StringId binding) noexcept
        {
            ActionMap* actionMap = actionMapPool.get(handle);

            if (actionMap == nullptr)
            {
                return std::nullopt;
            }

            return actionMap->getBindingState(binding);
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

    void InputActions::update(Authority<EngineCallbacks> auth) noexcept
    {
        m_pImpl->update();
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

    std::optional<InputSourceState> InputActions::getInputState(ActionMapHandle handle, StringId binding) noexcept
    {
        return m_pImpl->getInputState(handle, binding);
    }
}