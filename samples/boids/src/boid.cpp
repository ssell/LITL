#include <limits>

#include "litl-engine/scene/sceneView.hpp"
#include "litl-core/math/random.hpp"
#include "simulator.hpp"
#include "boid.hpp"
#include "food.hpp"

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
    void BoidSystem::update(EntityCommands& commands, float dt, Entity entity, Boid& boid, Transform& transform)
    {
        switch (boid.state)
        {
        case BoidState::Idle:
            onIdle(boid, transform);
            break;

        case BoidState::Traveling:
            onTraveling(boid, transform, dt);
            break;

        case BoidState::Fleeing:
            onFleeing(boid, transform);
            break;
        }
    }

    void BoidSystem::onIdle(Boid& boid, Transform& transform)
    {
        // Search for any nearby food.
        std::vector<Entity> foundFood; foundFood.reserve(8u);
        m_pSceneView->query<Food>(bounds::Sphere::fromCenterRadius(transform.getPosition(), 100.0f), foundFood);

        RandomFast rng{};

        if (!foundFood.empty())
        {
            // found food. move to the nearest one.
            float nearestDistSq = std::numeric_limits<float>::max();
            vec3 nearestFoodPos = {};

            for (auto foodEntity : foundFood)
            {
                auto foodPos = m_pSceneView->getWorldPosition(foodEntity);
                auto distSqToFood = transform.getPosition().distanceSqTo(foodPos);

                if (distSqToFood < nearestDistSq)
                {
                    nearestDistSq = distSqToFood;
                    nearestFoodPos = foodPos;
                }
            }

            boid.target = nearestFoodPos;
        }
        else
        {
            // move to a random target.
            boid.target = vec3(rng.next(m_worldSize), rng.next(m_worldSize), rng.next(m_worldSize));
        }

        boid.state = BoidState::Traveling;
    }

    void BoidSystem::onTraveling(Boid& boid, Transform& transform, float dt)
    {
        vec3 toTarget = (boid.target - transform.getPosition());
        vec3 toTargetDir = toTarget.normalized();

        if (toTarget.lengthSquared() <= 1.0f)
        {
            // ... nothing todo ...
        }
        else
        {
            transform.translate(toTargetDir * 10.0f * dt);
        }
    }

    void BoidSystem::onFleeing(Boid& boid, Transform& transform)
    {

    }
}