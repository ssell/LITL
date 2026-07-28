#include "litl-engine/scene/sceneView.hpp"
#include "boid.hpp"
#include "simulator.hpp"

namespace litl
{
    /// <summary>
    /// Invoked once at the start of the application, prior to the first frame.
    /// </summary>
    void BoidSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
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
        vec3 selfPos = transform.getPosition();

        if ((data.elapsedTime - boid.lastTick) > TickIntervalSec)
        {
            boid.lastTick = data.elapsedTime;
            boid.target = getTargetVector(boid, selfPos, movement);
        }

        vec3 steering = computeSteeringAcceleration(data.world, entity, selfPos, movement.velocity, boid.target);
        boid.acceleration = steering;
        boid.maxSpeed = g_boidSteering.maxSpeed;
    }

    /// <summary>
    /// Determines the boids target vector. This the vector directly to food or away from a predator.
    /// </summary>
    vec3 BoidSystem::getTargetVector(Boid& boid, vec3 selfPos, Movement const& movement)
    {
        // Each tick the boid is looking for two things: food and predators.
        // The boid searches for predators in a small radius, and if one is found it attempts to move away from them.
        // If there are no nearby predators, then the boid searches for the nearest food in a larger radius.
        // Finally, if there is no food, the boid continues on its current path until it nears the edge of the simulation.

        vec3 desiredVelocity = movement.velocity;       // if no food or predators found, will continue in current direction

        // 1. Check for predators.
        std::vector<PartitionQueryResult> findResults; findResults.reserve(8u);
        m_pSceneView->query<Predator>(bounds::Sphere::fromCenterRadius(selfPos, 50.0f), findResults, true);

        if (!findResults.empty())
        {
            vec3 awayFromPredator = (selfPos - findResults[0].worldPosition);
            if (awayFromPredator.isZeroed()) { awayFromPredator = vec3::right(); }

            desiredVelocity = awayFromPredator.normalized();
        }
        else
        {
            // 2. Check for food.
            findResults.clear();
            m_pSceneView->query<Food>(bounds::Sphere::fromCenterRadius(selfPos, 250.0f), findResults, true);

            if (!findResults.empty())
            {
                vec3 toFood = (findResults[0].worldPosition - selfPos);
                if (toFood.isZeroed()) { toFood = vec3::right(); }

                desiredVelocity = toFood.normalized();
            }
        }

        return desiredVelocity;
    }

    vec3 BoidSystem::computeSteeringAcceleration(World& world, Entity self, vec3 selfPos, vec3 selfVelocity, vec3 targetVector)
    {
        std::vector<PartitionQueryResult> neighbors; neighbors.reserve(8u);
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, g_boidSteering.perceptionRadius), neighbors, false);

        const float separationRadiusSq = g_boidSteering.separationRadius * g_boidSteering.separationRadius;

        vec3 accumulatedSeparation{};
        vec3 accumulatedVelocity{};
        vec3 accumulatedPosition{};
        uint32_t flockCount = 0u;

        for (auto& neighbor : neighbors)
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