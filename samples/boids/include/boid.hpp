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
        static constexpr uint32_t SteeringPhases = 8u;

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Boid& boid, Transform const& transform, Movement const& movement);

    private:

        void getTargetPosition(Boid& boid, vec3 selfPos);
        vec3 computeSteeringAcceleration(World& world, Entity self, vec3 selfPos, vec3 selfVelocity, vec3 targetVec);

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        std::shared_ptr<Simulator> m_pSimulator{ nullptr };
        uint32_t m_worldSize = 1024u;
    };
}

#endif