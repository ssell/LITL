#ifndef LITL_ENGINE_ECS_CULLING_SYSTEM_H__
#define LITL_ENGINE_ECS_CULLING_SYSTEM_H__

#include <memory>
#include <span>
#include <vector>
#include <unordered_set>

#include "litl-core/containers/flatHashSet.hpp"
#include "litl-engine/scene/sceneCameras.hpp"
#include "litl-engine/render/renderableEntity.hpp"

namespace litl
{
    class ServiceProvider;
    class EntityCommands;
    class SceneView;
    class Camera;

    /// <summary>
    /// A bucket of visible and renderable entities for each camera.
    /// </summary>
    struct alignas(Constants::cache_line_size) CullingBucket
    {
        /// <summary>
        /// All visible renderable entities for a specific camera.
        /// </summary>
        struct CameraRenderableEntities
        {
            Camera* camera = nullptr;
            std::vector<RenderableEntity> entities;
        };

        std::array<CameraRenderableEntities, SceneCameras::MaxSceneCameras> cameraRenderableEntities;
        void reset() noexcept;
    };

    /// <summary>
    /// Responsible for compiling a list of all renderable entities visible to each camera.
    /// </summary>
    class CullingSystem
    {
        /// <summary>
        /// All visible entities for each camera. 
        /// Note that visible renderable entities are a subset of this collection.
        /// </summary>
        struct CameraFrustumEntities
        {
            Camera* camera = nullptr;
            //FlatHashSet<Entity> entities;
            std::unordered_set<Entity> entities;
        };

    public:

        /// <summary>
        /// Injects the required services.
        /// </summary>
        /// <param name="services"></param>
        void setup(ServiceProvider& services);

        /// <summary>
        /// Resets the culling system state and computes the visible entities for each camera.
        /// The visible entities are later checked against our renderable entities to determine visible renderable entities.
        /// </summary>
        void prepare();

        /// <summary>
        /// Checks each renderable entity against the pre-computed set of visible entities for each camera.
        /// If a renderable entity is visible, it is added to a thread-specific collection that is later combined.
        /// </summary>
        void update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material);

        /// <summary>
        /// Retrieves all visible renderable entities for each camera.
        /// Will combine the per-thread results if they have not already been combined.
        /// </summary>
        /// <returns></returns>
        static CullingBucket const& getCombinedCullingBucket() noexcept;

    private:

        void reset() noexcept;
        void setCameraAtIndex(uint32_t i, Camera* camera) noexcept;

        std::shared_ptr<SceneView> m_pSceneView;
        std::vector<Entity> m_tempVisibleEntities;
        std::array<CameraFrustumEntities, SceneCameras::MaxSceneCameras> m_cameraVisibleEntities;

        static std::array<CullingBucket, Constants::max_thread_count> s_cullingBuckets;
        static CullingBucket s_combinedBucket;
        static bool s_combined;
        static thread_local uint32_t t_threadIndex;
    };
}

#endif