#ifndef LITL_SAMPLES_BOIDS_BOID_H__
#define LITL_SAMPLES_BOIDS_BOID_H__

#include <memory>
#include "components.hpp"

namespace litl
{
    class SceneView;
    class Simulator;

    class BoidSystem
    {
    public:

        static constexpr float TickIntervalSec = 0.25f;

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Boid& boid, Transform const& transform, Movement const& movement);

    private:

        vec3 getTargetPosition(vec3 selfPos);
        vec3 computeSteeringAcceleration(World& world, Entity self, vec3 selfPos, vec3 selfVelocity, vec3 targetVec);

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        uint32_t m_worldSize = 1024u;
    };
}

#endif