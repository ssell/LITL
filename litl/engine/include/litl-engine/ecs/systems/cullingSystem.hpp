#ifndef LITL_ENGINE_ECS_CULLING_SYSTEM_H__
#define LITL_ENGINE_ECS_CULLING_SYSTEM_H__

#include <memory>
#include <span>
#include <vector>

#include "litl-core/containers/flatHashSet.hpp"
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

    /// <summary>
    /// All visible renderable entities for a specific camera.
    /// </summary>
    struct CameraRenderableEntities
    {
        Camera* camera = nullptr;
        std::vector<Entity> entities;
    };

    /// <summary>
    /// A thread-specific bucket of visible and renderable entities for each camera.
    /// </summary>
    struct alignas(Constants::cache_line_size) CullingBucket
    {
        std::array<CameraRenderableEntities, SceneCameras::MaxSceneCameras> cameraRenderableEntities;
    };

    class CullingSystem
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material);

        static std::array<CameraRenderableEntities, SceneCameras::MaxSceneCameras>& getCameraRenderableEntities() noexcept;

    private:

        void reset() noexcept;
        void setCameraAtIndex(uint32_t i, Camera* camera) noexcept;

        struct CameraFrustumEntities
        {
            Camera* camera = nullptr;
            FlatHashSet<Entity> entities;
        };

        std::shared_ptr<SceneView> m_pSceneView;
        std::array<CameraFrustumEntities, SceneCameras::MaxSceneCameras> m_cameraVisibleEntities;
        std::array<CullingBucket, Constants::max_thread_count> m_cullingBuckets;
        std::vector<Entity> m_tempVisibleEntities;

        static thread_local uint32_t t_threadIndex;
    };
}

#endif