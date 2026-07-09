#include "litl-engine/ecs/systems/cullingSystem.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/entity/entityCommands.hpp"

namespace litl
{
    void CullingSystem::setup(ServiceProvider& services)
    {
        m_pSceneView = services.get<SceneView>();
    }

    void CullingSystem::prepare()
    {

    }

    void CullingSystem::update(EntityCommands& commands, float dt, Entity entity, Transform const& transform, MeshRef const& mesh, MaterialRef const& material)
    {

    }
}