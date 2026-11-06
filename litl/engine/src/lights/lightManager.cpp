#include <array>
#include <variant>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-core/job/perJobVector.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/engine.hpp"
#include "litl-engine/engineCallbacks.hpp"
#include "litl-engine/lights/lightManager.hpp"
#include "litl-engine/lights/lightData.hpp"
#include "litl-engine/lights/directional/directionalLight.hpp"
#include "litl-engine/lights/directional/directionalLightSystem.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    namespace
    {
        enum class LightCommandType
        {
            Unknown = 0u,
            MarkDirectionalActive = 1u
        };

        struct MarkDirectionalActiveCommand
        {
            Entity entity{};
            DirectionalLight light{};
            vec3 toLightDir{};
        };

        using LightCommandVar = std::variant<
            std::monostate,                         // LightCommandType::Unknown
            MarkDirectionalActiveCommand            // LightCommandType::MarkDirectionalActive
        >;

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

        struct DirectionalLightDataInfo
        {
            std::vector<DirectionalLightData> data{};
            std::vector<Entity> entities{};
            GpuBufferHandle handle{};

            void reserve(size_t count) noexcept
            {
                data.reserve(count);
                entities.reserve(count);
            }

            void clear() noexcept
            {
                data.clear();
                entities.clear();
            }
        };
    }

    struct LightManager::Impl
    {
        std::shared_ptr<ObjectPool> objectPool{ nullptr };
        std::shared_ptr<World> world{ nullptr };

        LightDataInfo lightData{};
        DirectionalLightDataInfo directionalLightData{};

        PerJobVector<LightCommand> commands;
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

            commands.reserve_each(32ull);
            directionalLightData.reserve(32ull);
        }

        void destroy() noexcept
        {
            // ... todo if needed ...
        }

        void onPreRender(uint32_t framesInFlight, uint32_t framesInFlightIndex) noexcept
        {
            directionalLightData.clear();

            processDeferredCommands();

            updateDirectionalLightDataBuffer(framesInFlightIndex);
            updateLightDataBuffer(framesInFlight, framesInFlightIndex);
        }

        // ---------------------------------------------------------------------------------
        // Command Processing
        // ---------------------------------------------------------------------------------

        template<typename T>
        void enqueueCommand(LightCommandType type, T command) noexcept
        {
            commands.push_back(LightCommand{
                .type = type,
                .command = command
            });
        }

        void processDeferredCommands() noexcept
        {
            commands.drain_into(commandsCopy);

            for (const auto& command : commandsCopy)
            {
                switch (command.type)
                {
                case LightCommandType::MarkDirectionalActive:
                {
                    const MarkDirectionalActiveCommand* ptr = std::get_if<MarkDirectionalActiveCommand>(&command.command);

                    if (ptr != nullptr)
                    {
                        processMarkDirectionalLightActiveCommand(*ptr);
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

        void processMarkDirectionalLightActiveCommand(MarkDirectionalActiveCommand const& command) noexcept
        {
            directionalLightData.data.push_back(DirectionalLightData{
                .colorIntensity = vec4(command.light.color.r(), command.light.color.g(), command.light.color.b(), command.light.intensity),
                .toLight = command.toLightDir,
                .shadowIndex = Constants::uint32_null_index
            });

            directionalLightData.entities.push_back(command.entity);
        }

        // ---------------------------------------------------------------------------------
        // Updates
        // ---------------------------------------------------------------------------------

        GpuBuffer* getDirectionalLightDataBuffer() noexcept
        {
            if (!directionalLightData.handle.isValid())
            {
                // Lazily create the buffer as the renderer may not be render when Setup is called.
                directionalLightData.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                    .objectInfo = ObjectDescriptor{.name = "LITL_INTERNAL_Buffer_DirectionalLightData" },
                    .type = BufferTypeFlagBits::BufferDeviceAddress,
                    .memoryUsage = BufferMemoryUsage::PersistentMap,
                    .bufferStrategy = GpuBufferingStrategy::Frame,
                    .bytes = sizeof(DirectionalLightData) * directionalLightData.data.size(),
                    .itemBytes = sizeof(DirectionalLightData),
                    .canResize = true
                    });

                LITL_FATAL_ASSERT_MSG(directionalLightData.handle.isValid(), "Failed to create Directional Light Data buffer.");
            }

            return objectPool->getGpuBuffer(directionalLightData.handle);
        }

        void updateDirectionalLightDataBuffer(uint32_t framesInFlightIndex) noexcept
        {
            if (directionalLightData.data.empty())
            {
                lightData.data.directionalLightsAddr = 0ull;
                lightData.data.directionalLightsCount = 0u;
                return;
            }

            auto* directionalLightDataBuffer = getDirectionalLightDataBuffer();
            LITL_ASSERT_MSG((directionalLightDataBuffer != nullptr), "Failed to retrieve Directional Light Data buffer.", );

            directionalLightDataBuffer->swapBuffers(framesInFlightIndex);

            if (directionalLightDataBuffer->getItemCapacity() < directionalLightData.data.size())
            {
                if (!directionalLightDataBuffer->resizeItems(directionalLightData.data.size(), 2u, false, true))
                {
                    logError("Failed to resize Directional Light Data buffer.");
                    return;
                }
            }

            directionalLightDataBuffer->recordChunkDataWrite(GpuBufferChunk{
                .sourcePtr = generic_as_byte_span(directionalLightData.data.data(), sizeof(DirectionalLightData)),
                .offset = 0ull
                });

            lightData.data.directionalLightsAddr = directionalLightDataBuffer->getBufferDeviceAddress().value();
            lightData.data.directionalLightsCount = static_cast<uint32_t>(directionalLightData.data.size());
        }

        GpuBuffer* getLightDataBuffer() noexcept
        {
            if (!lightData.handle.isValid())
            {
                // Lazily create the buffer as the renderer may not be render when Setup is called.
                lightData.handle = objectPool->createGpuBuffer(GpuBufferDescriptor{
                    .objectInfo = ObjectDescriptor{ .name = "LITL_INTERNAL_Buffer_LightData" },
                    .type = BufferTypeFlagBits::BufferDeviceAddress,
                    .memoryUsage = BufferMemoryUsage::PersistentMap,
                    .bufferStrategy = GpuBufferingStrategy::Frame,
                    .bytes = sizeof(LightData),
                    .itemBytes = sizeof(LightData)
                });

                LITL_FATAL_ASSERT_MSG(lightData.handle.isValid(), "Failed to create Light Data buffer.");
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

    void LightManager::setAmbientColor(color rgb) noexcept
    {
        m_pImpl->lightData.data.ambientColor = rgb;
        m_pImpl->lightData.dirty = true;
    }

    void LightManager::markActive(Authority<DirectionalLightSystem> auth, Entity entity, DirectionalLight const& light, vec3 toLightDir) noexcept
    {
        m_pImpl->enqueueCommand(LightCommandType::MarkDirectionalActive, MarkDirectionalActiveCommand{
            .entity = entity,
            .light = light,
            .toLightDir = toLightDir
        });
    }

    GpuBuffer const* LightManager::getLightDataBuffer() const noexcept
    {
        return m_pImpl->getLightDataBuffer();
    }

    uint32_t LightManager::getDirectionalLightCount() const noexcept
    {
        return static_cast<uint32_t>(m_pImpl->directionalLightData.data.size());
    }

    std::vector<DirectionalLightDataEntity> LightManager::getDirectionalLightData() const noexcept
    {
        std::vector<DirectionalLightDataEntity> lights;
        lights.reserve(m_pImpl->directionalLightData.data.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_pImpl->directionalLightData.data.size()); ++i)
        {
            lights.push_back(DirectionalLightDataEntity{
                .data = m_pImpl->directionalLightData.data[i],
                .entity = m_pImpl->directionalLightData.entities[i]
            });
        }

        return lights;
    }
}