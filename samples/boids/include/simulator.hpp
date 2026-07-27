#ifndef LITL_SAMPLES_BOIDS_SIMULATOR_H__
#define LITL_SAMPLES_BOIDS_SIMULATOR_H__

#include <chrono>
#include <cstdint>
#include <memory>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/math/random.hpp"
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    class ObjectPool;
    class World;

    struct SimulatorConfiguration
    {
        uint32_t worldDimensions = 1024u;
        uint32_t tickRateMs = 500u;
        uint32_t minBoidCount = 100u;
        uint32_t maxBoidCount = 100u;
        uint32_t minPredatorCount = 1u;
        uint32_t maxPredatorCount = 1u;
    };

    /// <summary>
    /// The simulator for the boids sample.
    /// Responsible for spawning food and additional boids if the population gets low.
    /// </summary>
    class Simulator
    {
    public:

        void setup(ServiceProvider& services, SimulatorConfiguration const& config) noexcept;
        void update(float dt) noexcept;

    private:

        void tick() noexcept;
        void spawnBoid() noexcept;
        void spawnPredator() noexcept;
        vec3 getRandomSpawnPoint() const noexcept;

        /// <summary>
        /// The configuration used to setup the simulation.
        /// </summary>
        SimulatorConfiguration m_config{};

        /// <summary>
        /// The global shared object pool. Could request each frame via services, but can also just keep a reference.
        /// </summary>
        std::shared_ptr<ObjectPool> m_pObjectPool{ nullptr };

        /// <summary>
        /// The global shared ECS world. Could request each frame via services, but can also just keep a reference.
        /// </summary>
        std::shared_ptr<World> m_pWorld{ nullptr };

        /// <summary>
        /// The time point that tick() was last invoked.
        /// </summary>
        std::chrono::steady_clock::time_point m_lastTick;

        /// <summary>
        /// The shared material used by all boids.
        /// </summary>
        MaterialHandle m_boidMaterial{};

        /// <summary>
        /// The shared mesh used by all boids.
        /// </summary>
        MeshHandle m_boidMesh{};

        /// <summary>
        /// The current number of boids in the simulation.
        /// </summary>
        uint32_t m_boidCount = 0u;

        /// <summary>
        /// The current number of predators in the simulation.
        /// </summary>
        uint32_t m_predatorCount = 0u;
    };
}

#endif