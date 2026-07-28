#include <limits>

#include "litl-engine/scene/sceneView.hpp"
#include "litl-core/math/random.hpp"
#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"
#include "predator.hpp"

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
    void BoidSystem::update(SystemData const& data, Entity entity, Boid& boid, Transform& transform, Movement& movement)
    {
        if ((data.elapsedTime - boid.lastTick) < 0.5f)
        {
            return;
        }

        boid.lastTick = data.elapsedTime;

        // Each tick the boid is looking for two things: food and predators.
        // The boid searches for predators in a small radius, and if one is found it attempts to move away from them.
        // If there are no nearby predators, then the boid searches for the nearest food in a larger radius.
        // Finally, if there is no food, the boid continues on its current path until it nears the edge of the simulation.

        boid.state = BoidState::Searching;       // generic searching state: looking for food or predators
        std::vector<PartitionQueryResult> findResults; findResults.reserve(8u);

        // 1. Check for predators.
        m_pSceneView->query<Predator>(bounds::Sphere::fromCenterRadius(transform.getPosition(), 10.0f), findResults, true);

        if (!findResults.empty())
        {
            vec3 awayFromPredator = (transform.getPosition() - findResults[0].worldPosition);
            if (awayFromPredator.isZeroed()) { awayFromPredator = vec3::right(); }

            movement.direction = awayFromPredator.normalized();
            boid.state = BoidState::Fleeing;
        }

        findResults.clear();

        // 2. Check for food.
        m_pSceneView->query<Food>(bounds::Sphere::fromCenterRadius(transform.getPosition(), 250.0f), findResults, true);

        if (!findResults.empty())
        {
            vec3 toFood = (findResults[0].worldPosition - transform.getPosition());

            if (toFood.isZeroed())
            {
                toFood = vec3::right();
                movement.speed = 0.0f;
            }
            else
            {
                movement.speed = Boid::BoidMovementSpeed;
            }

            movement.direction = toFood.normalized();
            boid.state = BoidState::Traveling;
        }
    }
}