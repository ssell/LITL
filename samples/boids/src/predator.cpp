#include "litl-engine/scene/sceneView.hpp"
#include "predator.hpp"
#include "simulator.hpp"

namespace litl::samples
{
    namespace
    {
        /// <summary>
        /// Use a shared vector for each thread. Allocations will stop once a high-water mark is reached.
        /// </summary>
        static thread_local std::vector<litl::PartitionQueryResult> t_partitionQueryResults;

        /// <summary>
        /// For each thread, we accumulate the number of consumed boids. That is then flattened in the prepare method.
        /// </summary>
        static thread_local std::array<uint32_t, litl::Constants::max_thread_count> t_consumedBoidCounts;
    }

    void PredatorSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
        m_pSimulator = services.get<Simulator>();
        m_worldSize = m_pSimulator->getConfig().worldDimensions;
        t_consumedBoidCounts.fill(0u);
    }

    void PredatorSystem::prepare()
    {
        // While update is called potentially on many threads in paralle, prepare is called just once per frame on the main thread.
        // As such it is a useful spot to do a pre-update pass (or in this case post, as we handle data from last frame) during a safe sync point.
        // Since t_consumedBoidCounts is populated in parallel, we take this chance to flatten into a total final count and feed it to the simulator.
        // This lets us avoid any slow sync mechanisms.

        uint32_t totalConsumedBoidsLastFrame = 0u;

        for (auto consumedPerThread : t_consumedBoidCounts)
        {
            totalConsumedBoidsLastFrame += consumedPerThread;
        }

        t_consumedBoidCounts.fill(0u);

        if (totalConsumedBoidsLastFrame > 0u)
        {
            m_pSimulator->updateBoidsConsumed(totalConsumedBoidsLastFrame);
        }
    }

    void PredatorSystem::update(SystemData const& data, Entity entity, Predator& predator, Acceleration& acceleration, Transform const& transform, Movement const& movement)
    {
        vec3 selfPos = transform.getPosition();
        m_pSimulator->updatePredatorPosition(predator.index, selfPos);

        // Update current target
        if ((data.elapsedTime - predator.lastTick) > TickIntervalSec)
        {
            predator.lastTick = data.elapsedTime;

            if (!predator.movingToTarget)
            {
                getTargetPosition(predator, selfPos);
            }
        }

        vec3 toTarget = predator.target - selfPos;
        vec3 toTargetDir = (toTarget.isZeroed() ? vec3::right() : toTarget.normalized());

        // Has the predator reached their target?        
        if (predator.movingToTarget && (toTarget.lengthSquared() <= 9.0f))
        {
            predator.movingToTarget = false;
        }

        // Move to target position
        vec3 steering = steerTowards(toTarget, movement.velocity, g_predatorSteering);
        acceleration.acceleration = truncate(steering, g_predatorSteering.maxForce);
        acceleration.maxSpeed = g_predatorSteering.maxSpeed;

        // Is the predator intersecting any boids?
        consumeNearbyBoids(data, selfPos);
    }

    void PredatorSystem::getTargetPosition(Predator& predator, vec3 selfPos)
    {
        t_partitionQueryResults.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, 100.0f), t_partitionQueryResults, true, 32u);

        if (!t_partitionQueryResults.empty())
        {
            auto toBoidDir = (t_partitionQueryResults[0].worldPosition - selfPos).normalized();
            predator.target = clamp(selfPos + (toBoidDir * 500.0f), vec3{ static_cast<float>(m_worldSize) * 0.1f, 0.0f, static_cast<float>(m_worldSize) * 0.1f }, vec3{ static_cast<float>(m_worldSize) * 0.9f, 0.0f, static_cast<float>(m_worldSize) * 0.9f });
            predator.movingToTarget = true;
        }
        else
        {
            predator.target = vec3{ static_cast<float>(m_worldSize / 2u), 0.0f, static_cast<float>(m_worldSize / 2u) };
            predator.movingToTarget = false;
        }
    }

    void PredatorSystem::consumeNearbyBoids(SystemData const& data, vec3 selfPos)
    {
        t_partitionQueryResults.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, 8.0f), t_partitionQueryResults, 1u);

        if (!t_partitionQueryResults.empty())
        {
            t_consumedBoidCounts[data.threadIndex] += static_cast<uint32_t>(t_partitionQueryResults.size());

            for (auto& result : t_partitionQueryResults)
            {
                data.commands.destroyEntity(result.entity);
            }
        }
    }
}