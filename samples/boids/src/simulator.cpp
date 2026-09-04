#include <limits>
#include <optional>

#include "litl-core/file.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/objects/objectPool.hpp"

#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"
#include "predator.hpp"

namespace litl::samples
{
    namespace
    {
        struct Triangle
        {
            std::array<Vertex, 3> vertices;
            std::array<uint32_t, 3> indices;

            static Triangle build(float scale, color bottomColor, color topColor) noexcept
            {
                return Triangle{
                    .vertices = std::array<Vertex, 3>{
                        Vertex{.position = vec3{ -scale, 0.0f, 0.0f }},
                        Vertex{.position = vec3{ 0.0f, 0.0f, scale * 2.0f }},
                        Vertex{.position = vec3{ scale, 0.0f, 0.0f }}
                    },
                    .indices = std::array<uint32_t, 3>{ 0u, 1u, 2u }
                };
            }
        };

        vec3 getRandomSpawnPoint(RandomLCG& rng, uint32_t worldDimensions, uint32_t padding) noexcept
        {
            return vec3(
                static_cast<float>(rng.next(worldDimensions - (padding * 2u)) + padding),
                0.0f,
                static_cast<float>(rng.next(worldDimensions - (padding * 2u)) + padding));
        }

        vec3 getRandomSpawnDirection(RandomLCG& rng) noexcept
        {
            return vec3{ rng.next01() * 2.0f - 1.0f, 0.0f, rng.next01() * 2.0f - 1.0f }.normalized();
        }

        [[nodiscard]] DeferredEntity spawnEntity(EntityCommands& commands, Material* material, MeshHandle meshHandle, vec3 position, float uniformScale, color color, std::optional<vec3> velocity) noexcept
        {
            auto& rng = RandomFast::shared();
            auto entity = commands.createEntity();
            const auto materialRef = MaterialRef{ .handle = material->getHandle(), .slot = material->allocateSlot() };

            commands.addComponent<Transform>(entity, Transform::create(position, quat::identity(), uniformScale));
            commands.addComponent<LocalBounds>(entity, LocalBounds{});
            commands.addComponent<WorldBounds>(entity, WorldBounds{});
            commands.addComponent<MaterialRef>(entity, materialRef);
            commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshHandle });

            if (velocity.has_value())
            {
                commands.addComponent<Acceleration>(entity, Acceleration{});
                commands.addComponent<Movement>(entity, Movement{ .velocity = velocity.value() });
            }

            material->setColor("tint"_sid, color, materialRef.slot);

            return entity;
        }

        void spawnBoid(EntityCommands& commands, Material* material, MeshHandle meshHandle, RandomFast& rng, uint32_t worldDimensions, uint32_t& boidCount) noexcept
        {
            const auto boidEntity = spawnEntity(commands, material, meshHandle, getRandomSpawnPoint(rng, worldDimensions, 0u), 10.0f, colors::Purple, getRandomSpawnDirection(rng) * g_boidSteering.maxSpeed);
            commands.addComponent<Boid>(boidEntity, Boid{ .phase = boidCount % BoidSystem::SteeringPhases, .lastTick = -rng.next01() * BoidSystem::TickIntervalSec });                           // Boid system calculates targets at a set interval. Set random lastTick times so all the initial boids dont tick at the same time.
            boidCount++;
        }

        [[nodiscard]] vec3 spawnPredator(EntityCommands& commands, Material* material, MeshHandle meshHandle, RandomFast& rng, uint32_t worldDimensions, uint32_t predatorCount) noexcept
        {
            const auto position = getRandomSpawnPoint(rng, worldDimensions, 0u);
            const auto predatorEntity = spawnEntity(commands, material, meshHandle, position, 15.0f, colors::Orange, getRandomSpawnDirection(rng) * g_predatorSteering.maxSpeed);
            commands.addComponent<Predator>(predatorEntity, Predator{ .index = static_cast<uint32_t>(predatorCount), .lastTick = -rng.next01() * PredatorSystem::TickIntervalSec });

            return position;
        }

        [[nodiscard]] vec3 spawnFood(EntityCommands& commands, Material* material, MeshHandle meshHandle, RandomFast& rng, uint32_t worldDimensions, uint32_t index) noexcept
        {
            const auto position = getRandomSpawnPoint(rng, worldDimensions, 20u);
            const auto foodEntity = spawnEntity(commands, material, meshHandle, position, 7.5f, colors::Green, std::nullopt);
            commands.addComponent<Food>(foodEntity, Food{ .index = index, .lastTick = -rng.next01() * FoodSystem::TickIntervalSec });

            return position;
        }
    }

    void Simulator::setup(ServiceProvider& services, SimulatorConfiguration const& config) noexcept
    {
        LITL_FATAL_ASSERT_MSG(config.worldDimensions > 0u, "Invalid world dimensions.");
        LITL_FATAL_ASSERT_MSG(config.tickRateMs >= 100u, "Invalid simulator tick rate. Minimum rate of 100ms between ticks.");
        LITL_FATAL_ASSERT_MSG(config.boidCount > 0u, "Invalid boid count.");

        m_pAssetManager = services.get<AssetManager>();
        m_pObjectPool = services.get<ObjectPool>();
        m_pWorld = services.get<World>();
        m_config = config;
        m_trackedFood.resize(m_config.foodCount, {});
        m_trackedPredators.resize(m_config.predatorCount, {});

        auto boidTriangle = Triangle::build(4.0f, colors::Blue, colors::Red);
        auto foodTriangle = Triangle::build(4.0f, colors::Green, colors::Green);
        auto predatorTriangle = Triangle::build(8.0f, colors::Red, colors::Yellow);

        auto* materialAsset = m_pAssetManager->getMaterial("materials/flat");

        if (materialAsset == nullptr)
        {
            logError("Failed to retrieve Boid material.");
            return;
        }

        m_materialHandle = materialAsset->handle;

        auto* meshAsset = m_pAssetManager->getMesh("mesh/bunny");

        if (meshAsset == nullptr)
        {
            logError("Failed to retrieve Boid mesh.");
            return;
        }

        m_meshHandle = meshAsset->handle;

        tick();
    }

    void Simulator::update(float dt) noexcept
    {
        auto now = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastTick).count() >= m_config.tickRateMs)
        {
            tick();
            m_lastTick = now;
        }
    }

    void Simulator::updateBoidsConsumed(uint32_t count) noexcept
    {
        if (m_config.respawnBoids)
        {
            m_boidCount -= count;
        }
    }

    void Simulator::updateFoodConsumed(uint32_t index) noexcept
    {
        m_trackedFood[index].foodStatus = Food::Status::Eaten;
    }

    void Simulator::updatePredatorPosition(uint32_t index, vec3 position) noexcept
    {
        m_trackedPredators[index] = position;
    }

    NearestPoint Simulator::getNearestFood(vec3 pos) noexcept
    {
        float nearestSq = std::numeric_limits<float>::max();
        vec3 nearestPos{ static_cast<float>(m_config.worldDimensions / 2u), 0.0f, static_cast<float>(m_config.worldDimensions / 2u) };

        for (auto& trackedFood : m_trackedFood)
        {
            if (trackedFood.foodStatus == Food::Status::Alive)
            {
                float distSq = distanceSq(trackedFood.position, pos);

                if (distSq < nearestSq)
                {
                    nearestSq = distSq;
                    nearestPos = trackedFood.position;
                }
            }
        }

        return NearestPoint{ .position = nearestPos, .distanceSq = nearestSq };
    }

    NearestPoint Simulator::getNearestPredator(vec3 pos) noexcept
    {
        float nearestSq = std::numeric_limits<float>::max();
        vec3 nearestPos{};

        for (vec3 trackedPredator : m_trackedPredators)
        {
            float distSq = distanceSq(trackedPredator, pos);

            if (distSq < nearestSq)
            {
                nearestSq = distSq;
                nearestPos = trackedPredator;
            }
        }

        return NearestPoint{ .position = nearestPos, .distanceSq = nearestSq };
    }

    SimulatorConfiguration const& Simulator::getConfig() const noexcept
    {
        return m_config;
    }

    void Simulator::tick() noexcept
    {
        if (!m_materialHandle.isValid() || !m_meshHandle.isValid())
        {
            return;
        }

        auto& commands = m_pWorld->getCommandBuffer();
        auto& rng = RandomFast::shared();
        auto* material = m_pObjectPool->getMaterial(m_materialHandle);

        while (m_boidCount < m_config.boidCount)
        {
            spawnBoid(commands, material, m_meshHandle, rng, m_config.worldDimensions, m_boidCount);
        }

        while (m_predatorCount < m_config.predatorCount)
        {
            m_trackedPredators[m_predatorCount] = spawnPredator(commands, material, m_meshHandle, rng, m_config.worldDimensions, m_predatorCount);
            m_predatorCount++;
        }

        // Update for any eaten food since the last tick
        for (size_t i = 0ull; i < m_trackedFood.size(); ++i)
        {
            if (m_trackedFood[i].foodStatus == Food::Status::Eaten)
            {
                m_trackedFood[i].foodStatus = Food::Status::None;
                m_foodCount--;
            }
        }

        while (m_foodCount < m_config.foodCount)
        {
            uint32_t nextIndex = Constants::uint32_null_index;

            for (uint32_t i = 0u; i < static_cast<uint32_t>(m_trackedFood.size()); ++i)
            {
                if (m_trackedFood[i].foodStatus == Food::Status::None)
                {
                    nextIndex = i;
                    break;
                }
            }

            if (nextIndex != Constants::uint32_null_index)
            {
                m_trackedFood[nextIndex].position = spawnFood(commands, material, m_meshHandle, rng, m_config.worldDimensions, nextIndex);
                m_trackedFood[nextIndex].foodStatus = Food::Status::Alive;
                m_foodCount++;
            }
        }
    }
}