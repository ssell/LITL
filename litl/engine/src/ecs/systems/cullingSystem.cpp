#include <atomic>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/objects/camera.hpp"

namespace litl
{
    namespace
    {
        std::atomic<uint32_t> g_nextThreadIndex{ 0 };
    }

    std::array<CullingBucket, Constants::max_thread_count> CullingSystem::s_cullingBuckets{};
    CullingBucket CullingSystem::s_combinedBucket{};
    bool CullingSystem::s_combined{ false };
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

        s_combinedBucket.reset();

        for (auto& cullingBucket : s_cullingBuckets)
        {
            cullingBucket.reset();
        }

        s_combined = false;
    }

    void CullingBucket::reset() noexcept
    {
        for (auto cameraRenderableEntities : cameraRenderableEntities)
        {
            cameraRenderableEntities.camera = nullptr;
            cameraRenderableEntities.entities.clear();
        }
    }

    void CullingSystem::setCameraAtIndex(uint32_t i, Camera* camera) noexcept
    {
        m_cameraVisibleEntities[i].camera = camera;
        s_combinedBucket.cameraRenderableEntities[i].camera = camera;

        for (auto& cullingBucket : s_cullingBuckets)
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
                s_cullingBuckets[t_threadIndex].cameraRenderableEntities[cameraIndex].entities.push_back(entity);
            }
        }
    }

    CullingBucket const& CullingSystem::getCombinedCullingBucket() noexcept
    {
        if (!s_combined)
        {
            // For each camera,
            for (uint32_t cameraIndex = 0u; cameraIndex < s_combinedBucket.cameraRenderableEntities.size(); ++cameraIndex)
            {
                auto& cameraRenderableEntities = s_combinedBucket.cameraRenderableEntities[cameraIndex];

                if (cameraRenderableEntities.camera == nullptr)
                {
                    // We have checked all cameras. Break out.
                    break;
                }

                // For each thread,
                for (auto& threadCullingBucket : s_cullingBuckets)
                {
                    // Add the visible renderable entities found for this camera for this thread
                    auto& threadRenderableEntities = threadCullingBucket.cameraRenderableEntities[cameraIndex].entities;
                    cameraRenderableEntities.entities.insert(cameraRenderableEntities.entities.end(), threadRenderableEntities.begin(), threadRenderableEntities.end());
                }
            }

            s_combined = true;
        }

        return s_combinedBucket;
    }
}