#ifndef LITL_SAMPLES_BOIDS_BOID_H__
#define LITL_SAMPLES_BOIDS_BOID_H__

#include <memory>

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "movement.hpp"

namespace litl
{
    enum class BoidState : uint8_t
    {
        Searching = 0u,
        Traveling = 1u,
        Fleeing = 2
    };

    struct Boid
    {
        static constexpr float BoidMovementSpeed = 50.0f;

        /// <summary>
        /// The current state of this boid.
        /// </summary>
        BoidState state{ BoidState::Searching };

        /// <summary>
        /// The fixed target that the boid is currently travelling to.
        /// </summary>
        vec3 target{};

        /// <summary>
        /// The last time tick was called for this boid.
        /// </summary>
        float lastTick{ 0.0f };
    };

    class SceneView;
    class Simulator;

    class BoidSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, Boid& boid, Transform& transform, Movement& movement);

    private:

        std::shared_ptr<SceneView> m_pSceneView{ nullptr };
        uint32_t m_worldSize = 1024u;
    };
}

LITL_REGISTER_COMPONENT(litl::Boid);

#endif