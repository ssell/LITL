#ifndef LITL_ENGINE_ECS_CULLING_SYSTEM_H__
#define LITL_ENGINE_ECS_CULLING_SYSTEM_H__

#include <memory>

#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"
#include "litl-engine/ecs/components/transform.hpp"

namespace litl
{
    class ServiceProvider;
    class EntityCommands;
    class SceneView;

    class CullingSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material);

    private:

        std::shared_ptr<SceneView> m_pSceneView;
    };
}

#endif