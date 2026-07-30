#include "litl-engine/scene/sceneView.hpp"
#include "food.hpp"
#include "simulator.hpp"

namespace litl::samples
{
    namespace
    {
        /// <summary>
        /// Use a shared vector for each thread for only a single allocation.
        /// </summary>
        static thread_local std::vector<PartitionQueryResult> t_partitionQueryResults;
    }

    void FoodSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
        m_pSimulator = services.get<Simulator>();
    }

    void FoodSystem::prepare()
    {
        // ... no action ...
    }

    void FoodSystem::update(SystemData const& data, Entity entity, Food& food, Transform const& transform)
    {
        vec3 selfPos = transform.getPosition();

        if ((data.elapsedTime - food.lastTick) < TickIntervalSec)
        {
            return;
        }

        food.lastTick = data.elapsedTime;

        t_partitionQueryResults.clear();
        m_pSceneView->query<Boid>(bounds::Sphere::fromCenterRadius(selfPos, 5.0f), t_partitionQueryResults, false, 1u);

        if (!t_partitionQueryResults.empty())
        {
            // Provide a food index that is unique to this instance. This allows us to alert the simulator in a thread-safe manner without any sync mechanisms.
            m_pSimulator->updateFoodConsumed(food.index);
            data.commands.destroyEntity(entity);
        }
    }
}