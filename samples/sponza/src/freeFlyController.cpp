#include "freeFlyController.hpp"

namespace litl::samples
{
    void FreeFlyControllerSystem::setup(ServiceProvider& services)
    {
        m_pInput = services.get<Input>();
        m_actionMapHandle = m_pInput->getMap("gameplay"_sid);
    }

    void FreeFlyControllerSystem::prepare()
    {

    }

    void FreeFlyControllerSystem::update(SystemData const& data, Entity entity, FreeFlyController& controller, Transform& transform)
    {
        updateTranslation(data, controller, transform);
        updateYawPitch(data, controller, transform);
    }

    void FreeFlyControllerSystem::updateTranslation(SystemData const& data, FreeFlyController const& controller, Transform& transform)
    {
        const auto speed = data.deltaTime * controller.speed;

        if (auto forwardState = m_pInput->getInputState(m_actionMapHandle, "forward"_sid); forwardState.has_value() && (forwardState.value().held == true))
        {
            transform.translate(transform.forward() * speed);
        }

        if (auto backwardState = m_pInput->getInputState(m_actionMapHandle, "backward"_sid); backwardState.has_value() && (backwardState.value().held == true))
        {
            transform.translate(-transform.forward() * speed);
        }

        if (auto leftState = m_pInput->getInputState(m_actionMapHandle, "left"_sid); leftState.has_value() && (leftState.value().held == true))
        {
            transform.translate(-transform.right() * speed);
        }

        if (auto rightState = m_pInput->getInputState(m_actionMapHandle, "right"_sid); rightState.has_value() && (rightState.value().held == true))
        {
            transform.translate(transform.right() * speed);
        }

        if (auto upState = m_pInput->getInputState(m_actionMapHandle, "up"_sid); upState.has_value() && (upState.value().held == true))
        {
            transform.translate(transform.up() * speed);
        }

        if (auto downState = m_pInput->getInputState(m_actionMapHandle, "down"_sid); downState.has_value() && (downState.value().held == true))
        {
            transform.translate(-transform.up() * speed);
        }
    }

    void FreeFlyControllerSystem::updateYawPitch(SystemData const& data, FreeFlyController& controller, Transform& transform)
    {
        const auto mouseDeltaState = m_pInput->getInputState(m_actionMapHandle, "look"_sid);

        if (!mouseDeltaState.has_value() || mouseDeltaState->xyDelta.isZeroed())
        {
            return;
        }

        vec3 eulerPYR = transform.getRotation().toEuler();

        eulerPYR.y() += mouseDeltaState->xyDelta.x() * controller.horizontalSensitivity;
        eulerPYR.x() += (mouseDeltaState->xyDelta.y() * controller.verticalSensitivity) * (controller.invertY ? -1.0f : 1.0f);
        eulerPYR.x() = clamp(eulerPYR.x(), controller.minPitch, controller.maxPitch);

        transform.setRotation(quat::fromEuler(eulerPYR));
    }
}