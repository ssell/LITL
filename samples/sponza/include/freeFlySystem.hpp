#ifndef LITL_SAMPLES_FREE_FLY_SYSTEM_H__
#define LITL_SAMPLES_FREE_FLY_SYSTEM_H__

#include <memory>

#include "litl-ecs/register.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-engine/input/inputActions.hpp"

namespace litl::samples
{
    struct FreeFly
    {
        float speed{ 1.0f };
    };

    struct FreeFlySystem final
    {
        void setup(ServiceProvider& services)
        {
            //m_pInputActions = services.get<InputActions>();
            //m_actionMapHandle = m_pInputActions->getMap("gameplay"_sid);
        }

        void prepare()
        {

        }

        void update(SystemData const& data, Entity entity, FreeFly const& freeFly, Transform& transform)
        {
            //auto forwardState = m_pInputActions->getInputState(m_actionMapHandle, "forward"_sid);
            //
            //if (forwardState.has_value() && (forwardState.value().held == true))
            //{
            //    transform.translate(vec3::forward() * data.deltaTime);
            //}
        }

    private:

        std::shared_ptr<InputActions> m_pInputActions{ nullptr };
        ActionMapHandle m_actionMapHandle{};
    };
}

LITL_REGISTER_COMPONENT(litl::samples::FreeFly);

#endif