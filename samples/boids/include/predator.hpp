#ifndef LITL_SAMPLES_BOIDS_PREDATOR_H__
#define LITL_SAMPLES_BOIDS_PREDATOR_H__

#include <memory>
#include "components.hpp"

namespace litl
{
    class SceneView;
}

namespace litl::samples
{
    class Simulator;

    /// <summary>
    /// Searches for boid targets and moves towards them.
    /// Any boids near enough to the predator are consumed.
    /// </summary>
    class PredatorSystem final
    {
    public:

        static constexpr float TickIntervalSec = 0.25f;

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Predator& predator, Acceleration& acceleration, Transform const& transform, Movement const& movement);

    private:

        void getTargetPosition(Predator& predator, vec3 selfPos);
        void consumeNearbyBoids(SystemData const& data, vec3 selfPos);

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        std::shared_ptr<Simulator> m_pSimulator{ nullptr };
        uint32_t m_worldSize = 1024u;
    };
}

#endif