#ifndef LITL_ENGINE_LIGHTS_DIRECTIONAL_LIGHT_SYSTEM_H__
#define LITL_ENGINE_LIGHTS_DIRECTIONAL_LIGHT_SYSTEM_H__

#include <memory>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/system/systemData.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/lights/directional/directionalLight.hpp"

namespace litl
{
    class LightManager;

    class DirectionalLightSystem final
    {
    public:

        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, DirectionalLight const& light, Transform const& transform);

    private:

        std::shared_ptr<LightManager> m_pLightManager{ nullptr };
    };
}

#endif