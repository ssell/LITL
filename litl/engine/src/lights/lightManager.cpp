#include <array>
#include <mutex>
#include <variant>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/containers/perThreadVector.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/lights/lightManager.hpp"
#include "litl-engine/lights/lightData.hpp"
#include "litl-engine/lights/directional/directionalLight.hpp"
#include "litl-engine/objects/objectPool.hpp"

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

        struct LightDataInfo
        {
            LightData data{};
            GpuBufferHandle handle{};
            bool dirty{ true };
            uint32_t dirtyFrames{ 0u };
        };
    }

    struct LightManager::Impl
    {
        std::shared_ptr<ObjectPool> objectPool{ nullptr };
        std::shared_ptr<World> world{ nullptr };

        LightDataInfo lightData{};

        std::mutex commandsMutex{};
        std::vector<LightCommand> commands;
        std::vector<LightCommand> commandsCopy;

        // ---------------------------------------------------------------------------------
        // Lifecycle
        // ---------------------------------------------------------------------------------

        void setup(ServiceProvider& services) noexcept
        {
            objectPool = services.get<ObjectPool>();
            world = services.get<World>();

            LITL_FATAL_ASSERT_MSG((objectPool != nullptr), "Failed to inject ObjectPool into LightManager");
            LITL_FATAL_ASSERT_MSG((world != nullptr), "Failed to inject World into LightManager");

            commands.reserve(32ull);
        }

        void destroy() noexcept
        {
            // ... todo if needed ...
        }

        void onPreRender(uint32_t framesInFlight, uint32_t framesInFlightIndex) noexcept
        {
            processDeferredCommands();
            updateLightDataBuffer(framesInFlight, framesInFlightIndex);
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

        // ---------------------------------------------------------------------------------
        // Updates
        // ---------------------------------------------------------------------------------

        GpuBuffer* getLightDataBuffer() noexcept
        {
            if (!lightData.handle.isValid())
            {
                // Lazily create the buffer as the renderer may not be render when Setup is called.
                lightData.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                    .objectInfo = ObjectDescriptor{.name = "LITL_INTERNAL_Buffer_LightData" },
                    .type = BufferTypeFlagBits::BufferDeviceAddress,
                    .memoryUsage = BufferMemoryUsage::PersistentMap,
                    .bufferStrategy = GpuBufferingStrategy::Frame,
                    .bytes = sizeof(LightData),
                    .itemBytes = sizeof(LightData)
                });

                LITL_FATAL_ASSERT_MSG(lightData.handle.isValid(), "Failedto create Light Data buffer.");
            }

            return objectPool->getGpuBuffer(lightData.handle);
        }

        void updateLightDataBuffer(uint32_t framesInFlight, uint32_t framesInFlightIndex) noexcept
        {
            auto* lightDataBuffer = getLightDataBuffer();
            LITL_ASSERT_MSG((lightDataBuffer != nullptr), "Failed to retrieve Light Data buffer.", );

            if (lightData.dirty)
            {
                lightData.dirtyFrames = framesInFlight;
                lightData.dirty = false;
            }

            lightDataBuffer->swapBuffers(framesInFlightIndex);

            if (lightData.dirtyFrames > 0u)
            {
                lightData.dirtyFrames--;

                lightDataBuffer->recordChunkDataWrite(GpuBufferChunk{
                    .sourcePtr = generic_as_byte_span(&lightData.data, sizeof(LightData)),
                    .offset = 0ull
                });
            }
        }
    };

    LightManager::LightManager()
        : m_pImpl(std::make_unique<LightManager::Impl>())
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

    void LightManager::onPreRender(Authority<EngineCallbacks> auth, uint32_t framesInFlight, uint32_t framesInFlightIndex) noexcept
    {
        m_pImpl->onPreRender(framesInFlight, framesInFlightIndex);
    }

    GpuBuffer const* LightManager::getLightDataBuffer() const noexcept
    {
        return m_pImpl->getLightDataBuffer();
    }

    void LightManager::setAmbientColor(color rgb) noexcept
    {
        m_pImpl->lightData.data.ambientColor = rgb;
        m_pImpl->lightData.dirty = true;
    }
}