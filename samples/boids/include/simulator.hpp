#ifndef LITL_SAMPLES_BOIDS_SIMULATOR_H__
#define LITL_SAMPLES_BOIDS_SIMULATOR_H__

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/math/random.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-engine/types/vertex.hpp"
#include "components.hpp"

namespace litl
{
    class ObjectPool;
    class World;
}

namespace litl::samples
{
    struct SimulatorConfiguration
    {
        uint32_t worldDimensions = 1024u;
        uint32_t tickRateMs = 500u;
        uint32_t boidCount = 100u;
        uint32_t predatorCount = 1u;
        uint32_t foodCount = 5u;
        float boidPredatorDetectionRadius = 50.0f;
        bool respawnBoids = false;
    };

    struct NearestPoint
    {
        vec3 position{};
        float distanceSq{ 0.0f };
    };

    /// <summary>
    /// The simulator for the boids sample.
    /// Responsible for spawning food and additional boids if the population gets low.
    /// </summary>
    class Simulator final
    {
    public:

        void setup(ServiceProvider& services, SimulatorConfiguration const& config) noexcept;
        void update(float dt) noexcept;
        void updateBoidsConsumed(uint32_t count) noexcept;
        void updateFoodConsumed(uint32_t index) noexcept;
        void updatePredatorPosition(uint32_t index, vec3 position) noexcept;

        [[nodiscard]] NearestPoint getNearestFood(vec3 pos) noexcept;
        [[nodiscard]] NearestPoint getNearestPredator(vec3 pos) noexcept;
        [[nodiscard]] SimulatorConfiguration const& getConfig() const noexcept;

    private:

        struct TrackedFood
        {
            vec3 position{};
            Food::Status foodStatus{ Food::Status::None };
        };

        void tick() noexcept;
        void spawnBoid() noexcept;
        void spawnPredator() noexcept;
        void spawnFood() noexcept;

        [[nodiscard]] MaterialHandle loadMaterial(std::span<char const> path, std::span<char const> name, std::span<char const> resource, std::span<char const> vertEntry, std::span<char const> fragEntry) const noexcept;
        [[nodiscard]] MeshHandle loadMesh(std::span<Vertex const> vertices, std::span<uint32_t const> indices, std::span<char const> name) const noexcept;

        /// <summary>
        /// The configuration used to setup the simulation.
        /// </summary>
        SimulatorConfiguration m_config{};

        /// <summary>
        /// Position and status of all food. Used to simplify the query for boids looking for food.
        /// </summary>
        std::vector<TrackedFood> m_trackedFood;

        /// <summary>
        /// Positions of all predators. Used to simplify the query for boids looking for predators.
        /// </summary>
        std::vector<vec3> m_trackedPredators;

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
        /// The shared material used by all.
        /// </summary>
        MaterialHandle m_sharedMaterial{};

        /// <summary>
        /// The mesh used by boids.
        /// </summary>
        MeshHandle m_boidMesh{};

        /// <summary>
        /// The mesh used by food.
        /// </summary>
        MeshHandle m_foodMesh{};

        /// <summary>
        /// The mesh used by predators.
        /// </summary>
        MeshHandle m_predatorMesh{};

        /// <summary>
        /// The current number of boids in the simulation.
        /// </summary>
        uint32_t m_boidCount = 0u;

        /// <summary>
        /// The current number of predators in the simulation.
        /// </summary>
        uint32_t m_predatorCount = 0u;

        /// <summary>
        /// The current number of foods in the simulation.
        /// </summary>
        uint32_t m_foodCount = 0u;
    };
}

#endif