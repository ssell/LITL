#include <array>
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
            Unknown = 0u
        };

        using LightCommandVar = std::variant<
            std::monostate                          // LightCommandType::Unknown
        >;        // LightCommandType::DestroyDirectional

        struct LightCommand
        {
            LightCommandType type{ LightCommandType::Unknown };
            LightCommandVar command{};
        };

        /// <summary>
        /// Individual directional light record which is uploaded to the GPU.
        /// This is built/updated each frame from DirectionalLight components added to entities.
        /// </summary>
        struct DirectionalLightRecord
        {
            color color{ colors::White };               // Color of the light.
            vec3 position{ vec3::zero() };              // World-space position of the light.
            vec3 lightDir{ vec3::forward() };           // Normalized directional vector that the light is travelling along.
            float intensity{ 1.0f };                    // Intensity/brightness of the light.
        };

        /// <summary>
        /// Block of directional lights that is uploaded to the GPU.
        /// </summary>
        struct DirectionalLightBlock
        {
            static constexpr uint32_t DirectionalLightsPerBlock = 32u;

            /// <summary>
            /// The raw byte data stored within this block. This is copied directly over to the GPU.
            /// </summary>
            std::vector<std::byte> data;
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
            // ... todo if needed ...
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
                case LightCommandType::Unknown:
                default:
                    break;
                }
            }

            commandsCopy.clear();
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
}