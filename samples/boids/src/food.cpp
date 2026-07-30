#include "litl-engine/scene/sceneView.hpp"
#include "food.hpp"
#include "simulator.hpp"

namespace litl::samples
{
    void FoodSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
        m_pSimulator = services.get<Simulator>();
    }

    void FoodSystem::prepare()
    {

    }

    void FoodSystem::update(SystemData const& data, Entity entity, Food& food, Transform const& transform)
    {
        vec3 selfPos = transform.getPosition();

        if ((data.elapsedTime - food.lastTick) > TickIntervalSec)
        {
            food.lastTick = data.elapsedTime;

            std::vector<PartitionQueryResult> nearbyBoids; nearbyBoids.reserve(8u);
            m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, 5.0f), nearbyBoids, false);

            if (!nearbyBoids.empty())
            {
                m_pSimulator->updateFoodConsumed(food.index);
                data.commands.destroyEntity(entity);
            }
        }
    }
}