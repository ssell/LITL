#include <mutex>
#include <ranges>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-platform/input/inputActions.hpp"
#include "litl-platform/input/input.hpp"
#include "litl-platform/input/actionMap.hpp"
#include "litl-platform/input/inputConfiguration.hpp"

namespace litl
{
    namespace
    {
        enum class InputActionCommandType : uint8_t
        {
            Unknown = 0u,
            Push = 1u,
            Pop = 2u
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
        std::vector<ActionMapHandle> allActionMapHandles;               // This is also tracked by actionMapPool but eliminates the need for us to call getAllHandles each frame since maps are static once they exist.
        std::vector<ActionMapHandle> actionMapStack;
        std::vector<InputActionCommand> writeActionMapCommands;
        std::vector<InputActionCommand> readActionMapCommands;
        std::vector<bool> actionMapProcessed;
        std::vector<ActionMapBinding> triggeredActionMaps;
        std::mutex commandsMutex{};
        ProcessedInputTracker processedInputTracker{};

        // ---------------------------------------------------------------------------------
        // Setup
        // ---------------------------------------------------------------------------------

        void setup(InputConfiguration const& config) noexcept
        {
            allActionMapHandles.reserve(config.actionMaps.size());
            writeActionMapCommands.reserve(32u);
            readActionMapCommands.reserve(32u);

            for (auto& descriptor : config.actionMaps)
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
                    continue;
                }

                actionMapLookup[StringId(descriptor.name)] = handle;
                allActionMapHandles.push_back(handle);
            }

            actionMapProcessed.resize(allActionMapHandles.size(), false);

            if (!config.defaultActionMap.empty())
            {
                pushMap(getMap(StringId(config.defaultActionMap)));
            }
        }

        // ---------------------------------------------------------------------------------
        // Update
        // ---------------------------------------------------------------------------------

        void update(Input const& input) noexcept
        {
            processCommands(input);

            updateActionsMaps();
            updateActionMapsWithInput(input);
            postUpdateActionMaps();
        }

        void processCommands(Input const& input) noexcept
        {
            readActionMapCommands.clear();

            {
                std::scoped_lock lock{ commandsMutex };
                std::swap(readActionMapCommands, writeActionMapCommands);
            }

            if (readActionMapCommands.empty())
            {
                return;
            }

            const ActionMapHandle prevTopHandle = (actionMapStack.empty() ? ActionMapHandle{} : actionMapStack.back());

            {
                for (auto& command : readActionMapCommands)
                {
                    processCommand(command);
                }
            }

            const ActionMapHandle currTopHandle = (actionMapStack.empty() ? ActionMapHandle{} : actionMapStack.back());

            if (prevTopHandle != currTopHandle)
            {
                const ActionMap* prevTop = actionMapPool.get(prevTopHandle);
                const ActionMap* currTop = actionMapPool.get(currTopHandle);

                const CursorMode prevCursorMode = (prevTop != nullptr) ? prevTop->getDescriptor().cursorMode : CursorMode::Normal;
                const CursorMode currCursorMode = (currTop != nullptr) ? currTop->getDescriptor().cursorMode : CursorMode::Normal;

                if (currCursorMode != prevCursorMode)
                {
                    input.setCursorMode(currCursorMode);
                }
            }
        }

        void updateActionsMaps() noexcept
        {
            for (auto actionMapHandle : allActionMapHandles)
            {
                auto* actionMap = actionMapPool.get(actionMapHandle);
                actionMap->onUpdate({});
            }
        }

        void updateActionMapsWithInput(Input const& input) noexcept
        {
            triggeredActionMaps.clear();

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

                if (!actionMap->updateInput({}, input, processedInputTracker))
                {
                    // This action map does not allow propagation of input down the stack.
                    break;
                }
            }
        }

        void postUpdateActionMaps() noexcept
        {
            for (auto actionMapHandle : allActionMapHandles)
            {
                auto* actionMap = actionMapPool.get(actionMapHandle);
                actionMap->onPostUpdate({}, triggeredActionMaps);
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

        void processCommandPush(InputActionCommand const& command) noexcept
        {
            actionMapStack.push_back(command.handle);
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
                actionMapStack.pop_back();
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

        void triggerCallbacks(ServiceProvider& services, InputCallbacks& callbacks) noexcept
        {
            if (triggeredActionMaps.empty())
            {
                return;
            }

            for (auto& triggeredActionMap : triggeredActionMaps)
            {
                const auto actionState = getActionState(getMap(triggeredActionMap.actionMapId), triggeredActionMap.bindingId);

                if (actionState.has_value() && actionState.value().triggered)
                {
                    callbacks.invokeActionMapEvent({}, services, triggeredActionMap.actionMapId, triggeredActionMap.bindingId, actionState.value());
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

            writeActionMapCommands.push_back(InputActionCommand{
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

            writeActionMapCommands.push_back(InputActionCommand{
                .handle = handle,
                .type = InputActionCommandType::Pop
                });
        }

        [[nodiscard]] std::optional<ActionMapState> getActionState(ActionMapHandle handle, StringId binding) noexcept
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

    void InputActions::setup(InputConfiguration const& config) noexcept
    {
        m_pImpl->setup(config);
    }

    void InputActions::update(Input const& input) noexcept
    {
        m_pImpl->update(input);
    }

    void InputActions::triggerCallbacks(ServiceProvider& services, InputCallbacks& callbacks) noexcept
    {
        m_pImpl->triggerCallbacks(services, callbacks);
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

    std::optional<ActionMapState> InputActions::getActionState(ActionMapHandle handle, StringId binding) noexcept
    {
        return m_pImpl->getActionState(handle, binding);
    }
}