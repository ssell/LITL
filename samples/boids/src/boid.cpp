#include "litl-engine/scene/sceneView.hpp"
#include "boid.hpp"
#include "simulator.hpp"

namespace litl
{
    namespace
    {
        static thread_local std::vector<PartitionQueryResult> t_neighbors;
    }

    /// <summary>
    /// Invoked once at the start of the application, prior to the first frame.
    /// </summary>
    void BoidSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
        m_pSimulator = services.get<Simulator>();
        m_worldSize = services.get<Simulator>()->getConfig().worldDimensions;
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
    void BoidSystem::update(SystemData const& data, Entity entity, Boid& boid, Transform const& transform, Movement const& movement)
    {
        if ((data.frameIndex % SteeringPhases) != boid.phase)
        {
            return;
        }

        vec3 selfPos = transform.getPosition();

        if ((data.elapsedTime - boid.lastTick) > TickIntervalSec)
        {
            boid.lastTick = data.elapsedTime;
            boid.target = getTargetPosition(selfPos);
        }

        vec3 targetDir = (boid.target - selfPos).normalized();
        vec3 steering = computeSteeringAcceleration(data.world, entity, selfPos, movement.velocity, targetDir);

        boid.acceleration = steering;
        boid.maxSpeed = g_boidSteering.maxSpeed;
    }

    /// <summary>
    /// Determines the boids target vector. This the vector directly to food or away from a predator.
    /// </summary>
    vec3 BoidSystem::getTargetPosition(vec3 selfPos)
    {
        std::vector<PartitionQueryResult> findResults; findResults.reserve(8u);
        m_pSceneView->query<Predator>(bounds::Sphere::fromCenterRadius(selfPos, 50.0f), findResults, true);

        if (!findResults.empty())
        {
            return findResults[0].worldPosition;
        }
        else
        {
            return m_pSimulator->getNearestFood(selfPos);
        }
    }

    vec3 BoidSystem::computeSteeringAcceleration(World& world, Entity self, vec3 selfPos, vec3 selfVelocity, vec3 targetVector)
    {
        t_neighbors.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, g_boidSteering.perceptionRadius), t_neighbors, false);

        const float separationRadiusSq = g_boidSteering.separationRadius * g_boidSteering.separationRadius;

        vec3 accumulatedSeparation{};
        vec3 accumulatedVelocity{};
        vec3 accumulatedPosition{};
        uint32_t flockCount = 0u;

        for (auto& neighbor : t_neighbors)
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

        acceleration += steerTowards(targetVector, selfVelocity, g_boidSteering) * g_boidSteering.targetWeight;

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