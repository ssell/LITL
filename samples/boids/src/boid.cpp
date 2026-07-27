#include "litl-engine/scene/sceneView.hpp"
#include "boid.hpp"

namespace litl
{
    /// <summary>
    /// Invoked once at the start of the application, prior to the first frame.
    /// </summary>
    void BoidSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
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
    void BoidSystem::update(EntityCommands& commands, float dt, Entity entity, Boid& boid, Transform const& transform)
    {
        switch (boid.state)
        {
        case BoidState::Idle:
            onIdle(boid, transform);
            break;

        case BoidState::Traveling:
            onTraveling(boid, transform);
            break;

        case BoidState::Fleeing:
            onFleeing(boid, transform);
            break;
        }
    }

    void BoidSystem::onIdle(Boid& boid, Transform const& transform)
    {
        // Search for any nearby food.
        //m_pSceneView->query(bounds::Sphere::fromCenterRadius(transform.getPosition(), 100.0f));
    }

    void BoidSystem::onTraveling(Boid& boid, Transform const& transform)
    {

    }

    void BoidSystem::onFleeing(Boid& boid, Transform const& transform)
    {

    }
}