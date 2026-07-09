#include <atomic>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/objects/camera/camera.hpp"

namespace litl
{
    namespace
    {
        std::atomic<uint32_t> g_nextThreadIndex{ 0 };
    }

    thread_local uint32_t CullingSystem::t_threadIndex = g_nextThreadIndex.fetch_add(1);

    void CullingSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
    }

    void CullingSystem::prepare()
    {
        // Perform frustum culling for each camera and repopulate the visible entities. Note that this is only a preprocess step - hence why this is a system. 
        // The frustum query gives all entities in the frustum with a transform. But all visible renderable entities is only a subset of that result.

        reset();

        auto cameras = m_pSceneView->getCameras();

        for (uint32_t i = 0u; (i < cameras.size()) && (i < m_cameraVisibleEntities.size()); ++i)
        {
            m_tempVisibleEntities.clear();

            auto* camera = cameras[i];
            auto& visibleEntities = m_cameraVisibleEntities[i];

            setCameraAtIndex(i, camera);

            m_pSceneView->query(camera->getFrustum(), m_tempVisibleEntities);

            for (auto entity : m_tempVisibleEntities)
            {
                visibleEntities.entities.insert(entity);
            }
        }
    }

    void CullingSystem::reset() noexcept
    {
        // Clear, but dont destroy, the camera entity containers. Eventually the cameras will reach a high-water mark of entities and the reallocations will stop.
        
        for (auto& cameraEntities : m_cameraVisibleEntities)
        {
            cameraEntities.camera = nullptr;
            cameraEntities.entities.clear();
        }

        for (auto& cullingBucket : m_cullingBuckets)
        {
            for (auto cameraRenderableEntities : cullingBucket.cameraRenderableEntities)
            {
                cameraRenderableEntities.camera = nullptr;
                cameraRenderableEntities.entities.clear();
            }
        }
    }

    void CullingSystem::setCameraAtIndex(uint32_t i, Camera* camera) noexcept
    {
        m_cameraVisibleEntities[i].camera = camera;

        for (auto& cullingBucket : m_cullingBuckets)
        {
            cullingBucket.cameraRenderableEntities[i].camera = camera;
        }
    }

    void CullingSystem::update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material)
    {
        for (uint32_t cameraIndex = 0u; cameraIndex < m_cameraVisibleEntities.size(); ++cameraIndex)
        {
            if (m_cameraVisibleEntities[cameraIndex].camera == nullptr)
            {
                // We have checked all cameras. Break out.
                break;
            }

            if (m_cameraVisibleEntities[cameraIndex].entities.contains(entity))
            {
                // We are visible to this camera, add to our thread-specific culling bucket.
                m_cullingBuckets[t_threadIndex].cameraRenderableEntities[cameraIndex].entities.push_back(entity);
            }
        }
    }
}