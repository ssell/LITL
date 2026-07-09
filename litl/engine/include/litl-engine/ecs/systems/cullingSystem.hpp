#ifndef LITL_ENGINE_ECS_CULLING_SYSTEM_H__
#define LITL_ENGINE_ECS_CULLING_SYSTEM_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/materialRef.hpp"
#include "litl-engine/ecs/components/meshRef.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/scene/sceneCameras.hpp"

namespace litl
{
    class ServiceProvider;
    class EntityCommands;
    class SceneView;
    class Camera;

    class CullingSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material);

    private:

        struct CameraFrustumEntities
        {
            Camera* camera = nullptr;
            std::vector<Entity> entities;
        };

        std::shared_ptr<SceneView> m_pSceneView;
        std::array<CameraFrustumEntities, SceneCameras::MaxSceneCameras> m_cameraVisibleEntities;
    };
}

#endif