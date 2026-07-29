#include <limits>

#include "litl-core/file.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/objects/objectPool.hpp"

#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"
#include "predator.hpp"

namespace litl
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
                        Vertex{.position = { -scale, 0.0f, 0.0f },       .color = bottomColor.rgb(), .uv = { 0.0f, 0.0f } },
                        Vertex{.position = { 0.0f, 0.0f, scale * 2.0f }, .color = topColor.rgb(),    .uv = { 0.5f, 1.0f } },
                        Vertex{.position = { scale, 0.0f, 0.0f },        .color = bottomColor.rgb(), .uv = { 1.0f, 0.0f } }
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
    }

    void Simulator::setup(ServiceProvider& services, SimulatorConfiguration const& config) noexcept
    {
        LITL_FATAL_ASSERT_MSG(config.worldDimensions > 0u, "Invalid world dimensions.");
        LITL_FATAL_ASSERT_MSG(config.tickRateMs >= 100u, "Invalid simulator tick rate. Minimum rate of 100ms between ticks.");
        LITL_FATAL_ASSERT_MSG(config.boidCount > 0u, "Invalid boid count.");

        m_pObjectPool = services.get<ObjectPool>();
        m_pWorld = services.get<World>();
        m_config = config;
        m_trackedFood.resize(m_config.foodCount, {});
        m_trackedPredators.resize(m_config.predatorCount, {});

        auto boidTriangle = Triangle::build(4.0f, colors::Blue, colors::Red);
        auto foodTriangle = Triangle::build(4.0f, colors::Green, colors::Green);
        auto predatorTriangle = Triangle::build(8.0f, colors::Red, colors::Yellow);

        m_boidMesh = loadMesh(boidTriangle.vertices, boidTriangle.indices, "Boid Mesh");
        m_foodMesh = loadMesh(foodTriangle.vertices, foodTriangle.indices, "Food Mesh");
        m_predatorMesh = loadMesh(predatorTriangle.vertices, predatorTriangle.indices, "Predator Mesh");
        m_sharedMaterial = loadMaterial("assets/shaders/spirv/flat.spv", "Boid Material", "flat.spv", "vertexMain", "fragmentMain");

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
        //m_boidCount -= count;
    }

    void Simulator::updateFoodConsumed(uint32_t index) noexcept
    {
        m_trackedFood[index].foodStatus = FoodStatus::Eaten;
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
            if (trackedFood.foodStatus == FoodStatus::Alive)
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
        while (m_boidCount < m_config.boidCount)
        {
            spawnBoid();
        }

        while (m_predatorCount < m_config.predatorCount)
        {
            spawnPredator();
        }

        // Update for any eaten food since the last tick
        for (size_t i = 0ull; i < m_trackedFood.size(); ++i)
        {
            if (m_trackedFood[i].foodStatus == FoodStatus::Eaten)
            {
                m_trackedFood[i].foodStatus = FoodStatus::None;
                m_foodCount--;
            }
        }

        while (m_foodCount < m_config.foodCount)
        {
            spawnFood();
        }
    }

    void Simulator::spawnBoid() noexcept
    {
        auto& commands = m_pWorld->getCommandBuffer();
        auto& rng = RandomFast::shared();
        auto boidEntity = commands.createEntity();

        commands.addComponent<Boid>(boidEntity, Boid{ .phase = m_boidCount % BoidSystem::SteeringPhases, .lastTick = -rng.next01() * BoidSystem::TickIntervalSec });                           // Boid system calculates targets at a set interval. Set random lastTick times so all the initial boids dont tick at the same time.
        commands.addComponent<Transform>(boidEntity, Transform::create(getRandomSpawnPoint(rng, m_config.worldDimensions, 0u)));
        commands.addComponent<LocalBounds>(boidEntity, LocalBounds{});
        commands.addComponent<WorldBounds>(boidEntity, WorldBounds{});
        commands.addComponent<Acceleration>(boidEntity, Acceleration{});
        commands.addComponent<Movement>(boidEntity, Movement{ .velocity = getRandomSpawnDirection(rng) * g_boidSteering.maxSpeed });
        commands.addComponent<MaterialRef>(boidEntity, MaterialRef{ .handle = m_sharedMaterial });
        commands.addComponent<MeshRef>(boidEntity, MeshRef{ .handle = m_boidMesh });

        m_boidCount++;
    }

    void Simulator::spawnPredator() noexcept
    {
        auto& commands = m_pWorld->getCommandBuffer();
        auto& rng = RandomFast::shared();
        auto predatorEntity = commands.createEntity();
        auto position = getRandomSpawnPoint(rng, m_config.worldDimensions, 0u);

        commands.addComponent<Predator>(predatorEntity, Predator{ .index = static_cast<uint32_t>(m_predatorCount), .lastTick = -rng.next01() * PredatorSystem::TickIntervalSec});
        commands.addComponent<Transform>(predatorEntity, Transform::create(position));
        commands.addComponent<LocalBounds>(predatorEntity, LocalBounds{});
        commands.addComponent<WorldBounds>(predatorEntity, WorldBounds{});
        commands.addComponent<Acceleration>(predatorEntity, Acceleration{});
        commands.addComponent<Movement>(predatorEntity, Movement{ .velocity = getRandomSpawnDirection(rng) * g_predatorSteering.maxSpeed });
        commands.addComponent<MaterialRef>(predatorEntity, MaterialRef{ .handle = m_sharedMaterial });
        commands.addComponent<MeshRef>(predatorEntity, MeshRef{ .handle = m_predatorMesh });

        m_trackedPredators[m_predatorCount] = position;
        m_predatorCount++;
    }

    void Simulator::spawnFood() noexcept
    {
        uint32_t nextIndex = Constants::uint32_null_index;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_trackedFood.size()); ++i)
        {
            if (m_trackedFood[i].foodStatus == FoodStatus::None)
            {
                nextIndex = i;
                break;
            }
        }

        if (nextIndex != Constants::uint32_null_index)
        {
            auto& commands = m_pWorld->getCommandBuffer();
            auto& rng = RandomFast::shared();
            auto foodEntity = commands.createEntity();
            auto position = getRandomSpawnPoint(rng, m_config.worldDimensions, 20u);

            commands.addComponent<Food>(foodEntity, Food{ .index = nextIndex, .lastTick = -rng.next01() * FoodSystem::TickIntervalSec });
            commands.addComponent<Transform>(foodEntity, Transform::create(position));
            commands.addComponent<LocalBounds>(foodEntity, LocalBounds{});
            commands.addComponent<WorldBounds>(foodEntity, WorldBounds{});
            commands.addComponent<MaterialRef>(foodEntity, MaterialRef{ .handle = m_sharedMaterial });
            commands.addComponent<MeshRef>(foodEntity, MeshRef{ .handle = m_foodMesh });

            m_trackedFood[nextIndex].position = position;
            m_trackedFood[nextIndex].foodStatus = FoodStatus::Alive;
            m_foodCount++;
        }
    }

    MaterialHandle Simulator::loadMaterial(std::span<char const> path, std::span<char const> name, std::span<char const> resource, std::span<char const> vertEntry, std::span<char const> fragEntry) const noexcept
    {
        auto spirvBytes = File(path).readAllBytes();

        return m_pObjectPool->createMaterial(MaterialDescriptor{
            .objectInfo = ObjectDescriptor {.name = name.data()},
            .vertexShader = ShaderResourceDescriptor {
                .resource = resource.data(),
                .entryPoint = vertEntry.data(),
                .bytes = spirvBytes.value()
            },
            .fragmentShader = ShaderResourceDescriptor {
                .resource = resource.data(),
                .entryPoint = fragEntry.data(),
                .bytes = spirvBytes.value()
            }
        });
    }

    MeshHandle Simulator::loadMesh(std::span<Vertex const> vertices, std::span<uint32_t const> indices, std::span<char const> name) const noexcept
    {
        return m_pObjectPool->createMesh(MeshDescriptor{
            .objectInfo = ObjectDescriptor {.name = name.data()},
            .vertexInfo = MeshVertexDescriptor {
                .vertexCount = static_cast<uint32_t>(vertices.size()),
                .vertexByteSize = sizeof(Vertex),
                .vertexData = as_byte_span(vertices)
            },
            .indexInfo = MeshIndexDescriptor {
                .indexCount = static_cast<uint32_t>(indices.size()),
                .indexByteSize = sizeof(uint32_t),
                .indexData = as_byte_span(indices)
            }
        });
    }
}