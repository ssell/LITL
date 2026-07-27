#ifndef LITL_SAMPLES_BOIDS_BOID_H__
#define LITL_SAMPLES_BOIDS_BOID_H__

#include <memory>

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"

namespace litl
{
    enum class BoidState : uint8_t
    {
        Idle = 0u,
        Traveling = 1u,
        Fleeing = 2
    };

    struct Boid
    {
        /// <summary>
        /// The current state of this boid.
        /// </summary>
        BoidState state{ BoidState::Idle };

        /// <summary>
        /// The fixed target that the boid is currently travelling to.
        /// </summary>
        vec3 target{};
    };

    class SceneView;
    class Simulator;

    class BoidSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(EntityCommands& commands, float dt, Entity entity, Boid& boid, Transform& transform);

    private:

        void onIdle(Boid& boid, Transform& transform);
        void onTraveling(Boid& boid, Transform& transform, float dt);
        void onFleeing(Boid& boid, Transform& transform);

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        uint32_t m_worldSize = 1024u;
    };
}

LITL_REGISTER_COMPONENT(litl::Boid);

#endif