#include "litl-engine/lights/directional/directionalLightSystem.hpp"
#include "litl-engine/lights/lightManager.hpp"

namespace litl
{
    void DirectionalLightSystem::setup(ServiceProvider& services)
    {
        m_pLightManager = services.get<LightManager>();
    }

    void DirectionalLightSystem::prepare()
    {

    }

    void DirectionalLightSystem::update(SystemData const& data, Entity entity, DirectionalLight const& light, Transform const& transform)
    {

    }
}