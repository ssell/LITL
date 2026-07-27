#include "simulator.hpp"
#include "boid.hpp"

#include "litl-core/file.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/objects/objectPool.hpp"

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
        m_pObjectPool = services.get<ObjectPool>();
        m_pWorld = services.get<World>();
        m_config = config;

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

    void Simulator::tick() noexcept
    {
        while (m_boidCount < m_config.minBoidCount)
        {
            spawnBoid();
        }

        while (m_predatorCount < m_config.maxPredatorCount)
        {
            spawnPredator();
        }
    }

    void Simulator::spawnBoid() noexcept
    {
        if (m_boidCount >= m_config.maxBoidCount)
        {
            return;
        }

        if (!m_boidMaterial.isValid())
        {
            auto spirvBytes = File("assets/shaders/spirv/flat.spv").readAllBytes();

            m_boidMaterial = m_pObjectPool->createMaterial(MaterialDescriptor{
                .objectInfo = ObjectDescriptor {.name = "Boid Material" },
                .vertexShader = ShaderResourceDescriptor {
                    .resource = "flat.spv",
                    .entryPoint = "vertexMain",
                    .bytes = spirvBytes.value()
                },
                .fragmentShader = ShaderResourceDescriptor {
                    .resource = "flat.spv",
                    .entryPoint = "fragmentMain",
                    .bytes = spirvBytes.value()
                }
            });
        }

        if (!m_boidMesh.isValid())
        {
            m_boidMesh = m_pObjectPool->createMesh(MeshDescriptor{
                .objectInfo = ObjectDescriptor { .name = "Boid Mesh" },
                .vertexInfo = MeshVertexDescriptor {
                    .vertexCount = 3u,
                    .vertexByteSize = sizeof(Vertex),
                    .vertexData = as_byte_span(s_boidVertices)
                },
                .indexInfo = MeshIndexDescriptor {
                    .indexCount = 3u,
                    .indexByteSize = sizeof(uint32_t),
                    .indexData = as_byte_span(s_boidIndices)
                }
            });
        }

        auto& commands = m_pWorld->getCommandBuffer();
        auto pos = getRandomSpawnPoint();
        auto boidEntity = commands.createEntity();
        commands.addComponent<Boid>(boidEntity, Boid{});
        commands.addComponent<Transform>(boidEntity, Transform::create(pos));
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

    vec3 Simulator::getRandomSpawnPoint() const noexcept
    {
        return vec3(
            static_cast<float>(Random::shared().next(m_config.worldDimensions)), 
            0.0f, 
            static_cast<float>(Random::shared().next(m_config.worldDimensions)));
    }
}