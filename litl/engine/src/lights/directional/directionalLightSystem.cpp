#include "litl-core/assert.hpp"
#include "litl-engine/lights/directional/directionalLightSystem.hpp"
#include "litl-engine/lights/lightManager.hpp"
#include "litl-engine/scene/sceneView.hpp"

namespace litl
{
    void DirectionalLightSystem::setup(ServiceProvider& services)
    {
        m_pLightManager = services.get<LightManager>();
        m_pSceneView = services.get<SceneView>();

        LITL_FATAL_ASSERT_MSG((m_pLightManager != nullptr), "Failed to inject LightManager into DirectionalLightSystem");
        LITL_FATAL_ASSERT_MSG((m_pSceneView != nullptr), "Failed to inject SceneView into DirectionalLightSystem");
    }

    void DirectionalLightSystem::prepare()
    {

    }

    void DirectionalLightSystem::update(SystemData const& data, Entity entity, DirectionalLight const& light, Transform const& transform)
    {
        // Make sure to use world-space forward and not local-space forward from the transform.
        const auto worldMatrix = m_pSceneView->getWorldMatrix(entity);
        m_pLightManager->markActive({}, entity, light, -worldMatrix.forward());
    }
}