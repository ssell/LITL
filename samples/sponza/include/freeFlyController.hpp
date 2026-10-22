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
        float speed{ 2.0f };                        // Base movement speed modifier.
        float sprint{ 3.0f };                       // Movement speed modifier when the "sprint" input is held.
        float horizontalSensitivity{ 0.085f };      // Modifier applied to rotation from "look".x
        float verticalSensitivity{ 0.065f };        // Modifier applied to rotation from "look".y
        float minPitch{ -90.0f };                   // Min pitch value.
        float maxPitch{ 90.0f };                    // Max pitch value.
        float yaw{ 0.0f };                          // Current horizontal rotation, in angles.
        float pitch{ 0.0f };                        // Current vertical rotation, in angles.
        bool invertY{ false };                      // Is the delta from "look".y inverted?
        bool initialized{ false };                  // Has this controller received its first update?
    };

    struct FreeFlyControllerSystem final
    {
        void setup(ServiceProvider& services);
        void prepare();
        void update(SystemData const& data, Entity entity, FreeFlyController& controller, Transform& transform);

    private:

        void initialize(FreeFlyController& controller, Transform const& transform);
        void updateTranslation(SystemData const& data, FreeFlyController const& controller, Transform& transform);
        void updateYawPitch(SystemData const& data, FreeFlyController& controller, Transform& transform);

        std::shared_ptr<Input> m_pInput{ nullptr };
        ActionMapHandle m_actionMapHandle{};
    };
}

LITL_REGISTER_COMPONENT(litl::samples::FreeFlyController);

#endif