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
            m_pInputActions = services.get<InputActions>();
            m_actionMapHandle = m_pInputActions->getMap("gameplay"_sid);
        }

        void prepare()
        {

        }

        void update(SystemData const& data, Entity entity, FreeFly const& freeFly, Transform& transform)
        {
            const auto speed = data.deltaTime * freeFly.speed;

            if (auto forwardState = m_pInputActions->getInputState(m_actionMapHandle, "forward"_sid); forwardState.has_value() && (forwardState.value().held == true))
            {
                transform.translate(transform.forward() * speed);
            }

            if (auto backwardState = m_pInputActions->getInputState(m_actionMapHandle, "backward"_sid); backwardState.has_value() && (backwardState.value().held == true))
            {
                transform.translate(-transform.forward() * speed);
            }

            if (auto leftState = m_pInputActions->getInputState(m_actionMapHandle, "left"_sid); leftState.has_value() && (leftState.value().held == true))
            {
                transform.translate(-transform.right() * speed);
            }

            if (auto rightState = m_pInputActions->getInputState(m_actionMapHandle, "right"_sid); rightState.has_value() && (rightState.value().held == true))
            {
                transform.translate(transform.right() * speed);
            }

            if (auto upState = m_pInputActions->getInputState(m_actionMapHandle, "up"_sid); upState.has_value() && (upState.value().held == true))
            {
                transform.translate(transform.up() * speed);
            }

            if (auto downState = m_pInputActions->getInputState(m_actionMapHandle, "down"_sid); downState.has_value() && (downState.value().held == true))
            {
                transform.translate(-transform.up() * speed);
            }
        }

    private:

        std::shared_ptr<InputActions> m_pInputActions{ nullptr };
        ActionMapHandle m_actionMapHandle{};
    };
}

LITL_REGISTER_COMPONENT(litl::samples::FreeFly);

#endif