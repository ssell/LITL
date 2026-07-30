#ifndef LITL_SAMPLES_BOID_FOOD_H__
#define LITL_SAMPLES_BOID_FOOD_H__

#include "components.hpp"

namespace litl
{
    class SceneView;
}

namespace litl::samples
{
    class Simulator;

    /// <summary>
    /// Checks for nearby boids and marks the food consumed if any are close enough.
    /// </summary>
    class FoodSystem final
    {
    public:

        static constexpr float TickIntervalSec = 0.15f;

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Food& food, Transform const& transform);

    private:

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        std::shared_ptr<Simulator> m_pSimulator{ nullptr };
    };
}

#endif