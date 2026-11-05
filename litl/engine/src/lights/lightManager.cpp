#include <mutex>
#include <variant>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/lights/lightManager.hpp"
#include "litl-engine/lights/directionalLight.hpp"

namespace litl
{
    namespace
    {
        enum class LightCommandType
        {
            Unknown = 0u,
            CreateDirectional = 1u,
            DestroyDirectional = 2u
        };

        struct CreateDirectionalLightCommand
        {
            DirectionalLightDescriptor descriptor{};
            DirectionalLightHandle handle{};
        };

        struct DestroyDirectionalLightCommand
        {
            DirectionalLightHandle handle{};
        };

        using LightCommandVar = std::variant<
            std::monostate,                         // LightCommandType::Unknown
            CreateDirectionalLightCommand,          // LightCommandType::CreateDirectional
            DestroyDirectionalLightCommand>;        // LightCommandType::DestroyDirectional

        struct LightCommand
        {
            LightCommandType type{ LightCommandType::Unknown };
            LightCommandVar command{};
        };
    }

    struct LightManager::Impl
    {
        HandlePool<DirectionalLightRecord, DirectionalLightHandleTag> directionalLightPool;

        std::mutex commandsMutex{};
        std::vector<LightCommand> commands;
        std::vector<LightCommand> commandsCopy;

        // ---------------------------------------------------------------------------------
        // Lifecycle
        // ---------------------------------------------------------------------------------

        void setup(ServiceProvider& services) noexcept
        {
            commands.reserve(32ull);
        }

        void destroy() noexcept
        {
            // ... todo ...
        }

        // ---------------------------------------------------------------------------------
        // Command Processing
        // ---------------------------------------------------------------------------------

        template<typename T>
        void enqueueCommand(LightCommandType type, T command) noexcept
        {
            std::scoped_lock lock{ commandsMutex };

            commands.push_back(LightCommand{
                .type = type,
                .command = command
            });
        }

        void processDeferredCommands() noexcept
        {
            {
                std::scoped_lock lock{ commandsMutex };
                commandsCopy = std::move(commands);
                commands.clear();
            }

            for (const auto& command : commandsCopy)
            {
                switch (command.type)
                {
                case LightCommandType::CreateDirectional:
                {
                    auto* commandPtr = std::get_if<CreateDirectionalLightCommand>(&command.command);

                    if (commandPtr != nullptr)
                    {
                        createDirectionalLight(*commandPtr);
                    }

                    break;
                }

                case LightCommandType::DestroyDirectional:
                {
                    auto* commandPtr = std::get_if<DestroyDirectionalLightCommand>(&command.command);

                    if (commandPtr != nullptr)
                    {
                        destroyDirectionalLight(commandPtr->handle);
                    }

                    break;
                }

                case LightCommandType::Unknown:
                default:
                    break;
                }
            }

            commandsCopy.clear();
        }

        // ---------------------------------------------------------------------------------
        // Directional Lights
        // ---------------------------------------------------------------------------------

        void createDirectionalLight(CreateDirectionalLightCommand const& command)
        {
            if (!command.handle.isValid())
            {
                return;
            }

            // ... todo ...
        }

        void destroyDirectionalLight(DirectionalLightHandle handle) noexcept
        {
            if (!handle.isValid())
            {
                return;
            }

            // ... todo ...

            directionalLightPool.destroy(handle);
        }
    };

    LightManager::LightManager()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    LightManager::~LightManager()
    {

    }

    void LightManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        m_pImpl->setup(services);
    }

    void LightManager::destroy(Authority<Engine> auth) noexcept
    {
        m_pImpl->destroy();
    }

    void LightManager::onPreRender(Authority<EngineCallbacks> auth) noexcept
    {
        m_pImpl->processDeferredCommands();
    }

    DirectionalLightHandle LightManager::createDirectionalLight(DirectionalLightDescriptor const& descriptor, vec3 position, quat rotation) noexcept
    {
        DirectionalLightRecord record{};
        const DirectionalLightHandle handle = m_pImpl->directionalLightPool.create(record);

        if (!handle.isValid())
        {
            logError("Failed to allocate new Directional Light pooled resource.");
            return {};
        }

        m_pImpl->enqueueCommand(LightCommandType::CreateDirectional, CreateDirectionalLightCommand{
            .descriptor = descriptor,
            .handle = handle
        });

        return handle;
    }

    void LightManager::destroyDirectionalLight(DirectionalLightHandle handle) noexcept
    {
        m_pImpl->destroyDirectionalLight(handle);
    }

    void LightManager::deferDestroyDirectionalLight(DirectionalLightHandle handle) noexcept
    {
        if (!handle.isValid())
        {
            return;
        }

        m_pImpl->enqueueCommand(LightCommandType::DestroyDirectional, DestroyDirectionalLightCommand{ .handle = handle });
    }
}