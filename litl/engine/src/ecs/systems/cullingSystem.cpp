#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/entity/entityCommands.hpp"
#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/objects/camera/camera.hpp"


namespace litl
{
    void CullingSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
    }

    void CullingSystem::prepare()
    {
        // Clear, but dont destroy, the camera entity containers. Eventually the cameras will reach a high-water mark of entities and the reallocations will stop.
        for (auto& cameraEntities : m_cameraVisibleEntities)
        {
            cameraEntities.camera = nullptr;
            cameraEntities.entities.clear();
        }

        // Perform frustum culling for each camera and repopulate the visible entities. Note that this is only a preprocess step - hence why this is a system. 
        // The frustum query gives all entities in the frustum with a transform. But all visible renderable entities is only a subset of that result.
        auto cameras = m_pSceneView->getCameras();

        for (uint32_t i = 0u; (i < cameras.size()) && (i < m_cameraVisibleEntities.size()); ++i)
        {
            auto* camera = cameras[i];
            auto& visibleEntities = m_cameraVisibleEntities[i];

            visibleEntities.camera = camera;
            m_pSceneView->query(camera->getFrustum(), visibleEntities.entities);
        }
    }

    void CullingSystem::update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material)
    {

    }
}