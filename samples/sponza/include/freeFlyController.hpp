#ifndef LITL_SAMPLES_FREE_FLY_SYSTEM_H__
#define LITL_SAMPLES_FREE_FLY_SYSTEM_H__

#include <memory>

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-platform/input/input.hpp"

namespace litl::samples
{
    struct FreeFlyController
    {
        float speed{ 1.0f };
        float horizontalSensitivity{ 0.085f };
        float verticalSensitivity{ 0.065f };
        float minPitch{ -90.0f };
        float maxPitch{ 90.0f };
        bool invertY{ false };
    };

    struct FreeFlyControllerSystem final
    {
        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, FreeFlyController& controller, Transform& transform);

    private:

        void updateTranslation(SystemData const& data, FreeFlyController const& controller, Transform& transform);
        void updateYawPitch(SystemData const& data, FreeFlyController& controller, Transform& transform);

        std::shared_ptr<Input> m_pInput{ nullptr };
        ActionMapHandle m_actionMapHandle{};
    };
}

LITL_REGISTER_COMPONENT(litl::samples::FreeFlyController);

#endif