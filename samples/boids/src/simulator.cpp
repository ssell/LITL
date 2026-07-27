#include "litl-core/file.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/objects/objectPool.hpp"

#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"

namespace litl
{
    namespace
    {
        constexpr std::array<Vertex, 3> s_boidVertices = {
            Vertex {                                        // left
                .position = { -2.0f, 0.0f, 0.0f },
                .color = { 0.0f, 0.0f, 1.0f },
                .uv = { 0.0f, 0.0f }
            },
            Vertex {                                        // top
                .position = { 0.0f, 0.0f, 4.0f },
                .color = { 1.0f, 0.0f, 0.0f },
                .uv = { 0.5f, 1.0f }
            },
            Vertex {                                        // right
                .position = { 2.0f, 0.0f, 0.0f },
                .color = { 0.0f, 0.0f, 1.0f },
                .uv = { 1.0f, 0.0f }
            }
        };

        const std::array<uint32_t, 3> s_boidIndices = { 0, 1, 2 };
    }

    void Simulator::setup(ServiceProvider& services, SimulatorConfiguration const& config) noexcept
    {
        LITL_FATAL_ASSERT_MSG(config.worldDimensions > 0u, "Invalid world dimensions.");
        LITL_FATAL_ASSERT_MSG(config.tickRateMs >= 100u, "Invalid simulator tick rate. Minimum rate of 100ms between ticks.");
        LITL_FATAL_ASSERT_MSG(config.minBoidCount <= config.maxBoidCount, "Invalid min/max boid count.");
        LITL_FATAL_ASSERT_MSG(config.minPredatorCount <= config.maxPredatorCount, "Invalid min/max predator count.");

        m_pObjectPool = services.get<ObjectPool>();
        m_pWorld = services.get<World>();
        m_config = config;

        m_boidMaterial = loadMaterial("assets/shaders/spirv/flat.spv", "Boid Material", "flat.spv", "vertexMain", "fragmentMain");
        m_boidMesh = loadMesh(s_boidVertices, s_boidIndices, "Boid Mesh");

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

    SimulatorConfiguration const& Simulator::getConfig() const noexcept
    {
        return m_config;
    }

    void Simulator::tick() noexcept
    {
        while (m_boidCount < m_config.minBoidCount)
        {
            spawnBoid();
        }

        while (m_predatorCount < m_config.minPredatorCount)
        {
            spawnPredator();
        }

        while (m_foodCount < m_config.minFoodCount)
        {
            spawnFood();
        }
    }

    void Simulator::spawnBoid() noexcept
    {
        if (m_boidCount >= m_config.maxBoidCount)
        {
            return;
        }

        auto& commands = m_pWorld->getCommandBuffer();
        auto boidEntity = commands.createEntity();

        commands.addComponent<Boid>(boidEntity, Boid{});
        commands.addComponent<Transform>(boidEntity, Transform::create(getRandomSpawnPoint()));
        commands.addComponent<MaterialRef>(boidEntity, MaterialRef{ .handle = m_boidMaterial });
        commands.addComponent<MeshRef>(boidEntity, MeshRef{ .handle = m_boidMesh });

        m_boidCount++;
    }

    void Simulator::spawnPredator() noexcept
    {
        if (m_predatorCount >= m_config.maxPredatorCount)
        {
            return;
        }

        // ... todo ...

        m_predatorCount++;
    }

    void Simulator::spawnFood() noexcept
    {
        if (m_foodCount >= m_config.maxFoodCount)
        {
            return;
        }

        auto& commands = m_pWorld->getCommandBuffer();
        auto foodEntity = commands.createEntity();

        commands.addComponent<Food>(foodEntity, Food{});
        commands.addComponent<Transform>(foodEntity, Transform::create(getRandomSpawnPoint()));

        m_foodCount++;
    }

    vec3 Simulator::getRandomSpawnPoint() const noexcept
    {
        return vec3(
            static_cast<float>(Random::shared().next(m_config.worldDimensions)), 
            0.0f, 
            static_cast<float>(Random::shared().next(m_config.worldDimensions)));
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