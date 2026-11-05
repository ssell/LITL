#include <mutex>
#include <variant>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
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
        std::shared_ptr<World> world{ nullptr };

        HandlePool<DirectionalLightRecord, DirectionalLightHandleTag> directionalLightPool;

        std::mutex commandsMutex{};
        std::vector<LightCommand> commands;
        std::vector<LightCommand> commandsCopy;

        // ---------------------------------------------------------------------------------
        // Lifecycle
        // ---------------------------------------------------------------------------------

        void setup(ServiceProvider& services) noexcept
        {
            world = services.get<World>();

            LITL_FATAL_ASSERT_MSG((world != nullptr), "Failed to inkect World into LightManager");

            commands.reserve(32ull);
        }

        void destroy() noexcept
        {
            // ... todo ...
        }

        void onPreRender() noexcept
        {
            processDeferredCommands();
            // ... todo update GPU light data ...
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
                        destroyDirectionalLight(commandPtr->handle, true);
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

        void destroyLightEntity(Entity entity, bool atSyncPoint) noexcept
        {
            if (entity.isNull())
            {
                return;
            }

            if (atSyncPoint)
            {
                world->destroyImmediate(entity);
            }
            else
            {
                world->getCommandBuffer().destroyEntity(entity);
            }
        }

        // ---------------------------------------------------------------------------------
        // Directional Lights
        // ---------------------------------------------------------------------------------

        void createDirectionalLight(CreateDirectionalLightCommand const& command)           
        {
            DirectionalLightRecord* record = directionalLightPool.get(command.handle);

            if ((record == nullptr) || !record->entity.isNull())
            {
                return;
            }

            const DirectionalLight light{
                .color = command.descriptor.color,
                .intensity = command.descriptor.intensity
            };

            record->entity = world->createImmediate();
            world->addComponentImmediate<DirectionalLight>(record->entity, light);
        }

        void destroyDirectionalLight(DirectionalLightHandle handle, bool atSyncPoint) noexcept
        {
            DirectionalLightRecord* record = directionalLightPool.get(handle);

            if (record == nullptr)
            {
                return;
            }

            destroyLightEntity(record->entity, atSyncPoint);
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
        m_pImpl->onPreRender();
    }

    DirectionalLightHandle LightManager::createDirectionalLight(DirectionalLightDescriptor const& descriptor, vec3 position, quat rotation) noexcept
    {
        // Reminder: do not call m_pImpl->createDirectionalLight directly as it invokes world->createImmediate. It needs to be done during a sync point.

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
        m_pImpl->destroyDirectionalLight(handle, false);
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