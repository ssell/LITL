#include "litl-engine/scene/sceneView.hpp"
#include "boid.hpp"
#include "simulator.hpp"

namespace litl
{
    namespace
    {
        static thread_local std::vector<PartitionQueryResult> t_partitionQueryResults;
    }

    /// <summary>
    /// Invoked once at the start of the application, prior to the first frame.
    /// </summary>
    void BoidSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
        m_pSimulator = services.get<Simulator>();
        m_worldSize = m_pSimulator->getConfig().worldDimensions;
        m_predatorRadiusSq = m_pSimulator->getConfig().boidPredatorDetectionRadius * m_pSimulator->getConfig().boidPredatorDetectionRadius;
    }

    /// <summary>
    /// Invoked once per frame, prior to update being called.
    /// </summary>
    void BoidSystem::prepare()
    {

    }

    /// <summary>
    /// Invoked each frame for each entity that matches the required components. 
    /// Each valid chunk of relevant entities is run at the same time in parallel as other matching chunks.
    /// </summary>
    void BoidSystem::update(SystemData const& data, Entity entity, Boid& boid, Acceleration& acceleration, Transform const& transform, Movement const& movement)
    {
        if ((data.frameIndex % SteeringPhases) != boid.phase)
        {
            // Due to the number of boids, we do not need to update all of them every frame. Instead we can update each one once every SteeringPhases frames.
            return;
        }

        vec3 selfPos = transform.getPosition();

        if ((data.elapsedTime - boid.lastTick) > TickIntervalSec)
        {
            boid.lastTick = data.elapsedTime;
            getTargetPosition(boid, selfPos);
        }

        vec3 targetDir = (boid.target - selfPos).normalized() * (boid.isFleeing ? -1.0f : 1.0f);
        vec3 steering = computeSteeringAcceleration(data.world, entity, selfPos, movement.velocity, targetDir, boid.isFleeing);

        acceleration.acceleration = steering;
        acceleration.maxSpeed = g_boidSteering.maxSpeed;
    }

    /// <summary>
    /// Determines the boids target vector. This the vector directly to food or away from a predator.
    /// </summary>
    void BoidSystem::getTargetPosition(Boid& boid, vec3 selfPos)
    {
        NearestPoint nearestTargetPoint = m_pSimulator->getNearestPredator(selfPos);

        if (nearestTargetPoint.distanceSq <= m_predatorRadiusSq)
        {
            boid.target = nearestTargetPoint.position;
            boid.isFleeing = true;
        }
        else
        {
            boid.target = m_pSimulator->getNearestFood(selfPos).position;
            boid.isFleeing = false;
        }
    }

    vec3 BoidSystem::computeSteeringAcceleration(World& world, Entity self, vec3 selfPos, vec3 selfVelocity, vec3 targetVector, bool isFleeing)
    {
        t_partitionQueryResults.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, g_boidSteering.perceptionRadius), t_partitionQueryResults, false, 8u);

        const float separationRadiusSq = g_boidSteering.separationRadius * g_boidSteering.separationRadius;

        vec3 accumulatedSeparation{};
        vec3 accumulatedVelocity{};
        vec3 accumulatedPosition{};
        uint32_t flockCount = 0u;

        for (auto& neighbor : t_partitionQueryResults)
        {
            if (neighbor.entity == self)
            {
                continue;
            }

            const auto awayFromNeighbor = selfPos - neighbor.worldPosition;
            const auto distFromNeighborSq = awayFromNeighbor.lengthSquared();

            if (distFromNeighborSq < kEpsilonSq)
            {
                continue;
            }

            if (distFromNeighborSq < separationRadiusSq)
            {
                accumulatedSeparation += (awayFromNeighbor / distFromNeighborSq);     // normalize "away from neighbor" vector
            }

            accumulatedVelocity += world.getComponent<Movement>(neighbor.entity).value().velocity;
            accumulatedPosition += neighbor.worldPosition;
            flockCount++;
        }

        vec3 acceleration{};

        acceleration += steerTowards(targetVector, selfVelocity, g_boidSteering) * g_boidSteering.targetWeight * (isFleeing ? 16.0f : 1.0f);

        if (accumulatedSeparation.lengthSquared() > kEpsilonSq)
        {
            acceleration += steerTowards(accumulatedSeparation, selfVelocity, g_boidSteering) * g_boidSteering.separationWeight;
        }

        if (flockCount > 0u)
        {
            const float inverse = 1.0f / static_cast<float>(flockCount);
            acceleration += steerTowards(accumulatedVelocity * inverse, selfVelocity, g_boidSteering) * g_boidSteering.alignmentWeight;
            acceleration += steerTowards((accumulatedPosition * inverse) - selfPos, selfVelocity, g_boidSteering) * g_boidSteering.cohesionWeight;
        }

        return truncate(acceleration, g_boidSteering.maxForce);
    }
}