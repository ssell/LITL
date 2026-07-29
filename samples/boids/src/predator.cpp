#include "litl-engine/scene/sceneView.hpp"
#include "predator.hpp"
#include "simulator.hpp"

namespace litl
{
    namespace
    {
        static thread_local std::vector<PartitionQueryResult> t_partitionQueryResults;
        static thread_local std::array<uint32_t, Constants::max_thread_count> t_consumedBoidCounts;
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
        uint32_t totalConsumedBoidsLastFrame = 0u;

        for (auto consumedPerThread : t_consumedBoidCounts)
        {
            totalConsumedBoidsLastFrame += consumedPerThread;
        }

        t_consumedBoidCounts.fill(0u);

        if (totalConsumedBoidsLastFrame > 0u)
        {
            // ... alert simulator to spawn more ...
        }
    }

    void PredatorSystem::update(SystemData const& data, Entity entity, Predator& predator, Acceleration& acceleration, Transform const& transform, Movement const& movement)
    {
        vec3 selfPos = transform.getPosition();
        m_pSimulator->updatePredatorPosition(predator.index, selfPos);

        if ((data.elapsedTime - predator.lastTick) > TickIntervalSec)
        {
            predator.lastTick = data.elapsedTime;

            if (!predator.movingToTarget)
            {
                getTargetPosition(predator, selfPos);
            }
        }

        vec3 toTarget = predator.target - selfPos;
        vec3 toTargetDir = toTarget.normalized();

        // Has the predator reached their target?        
        if (predator.movingToTarget && (toTarget.lengthSquared() <= 9.0f))
        {
            predator.movingToTarget = false;
        }

        // Move to target position
        vec3 steering = steerTowards(toTarget, movement.velocity, g_predatorSteering);
        acceleration.acceleration = truncate(steering, g_predatorSteering.maxForce);
        acceleration.maxSpeed = g_predatorSteering.maxSpeed;

        // Is the predator intersection any boids?
    }

    void PredatorSystem::getTargetPosition(Predator& predator, vec3 selfPos)
    {
        t_partitionQueryResults.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, g_predatorSteering.perceptionRadius), t_partitionQueryResults, true, 32u);

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
}