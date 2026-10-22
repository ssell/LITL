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
        if (!controller.initialized)
        {
            initialize(controller, transform);
        }

        updateTranslation(data, controller, transform);
        updateYawPitch(data, controller, transform);
    }

    void FreeFlyControllerSystem::initialize(FreeFlyController& controller, Transform const& transform)
    {
        controller.initialized = true;

        // Set the initial yaw/pitch to match the direction the entity is currently facing.
        const vec3 forward = transform.forward();
        controller.yaw = radiansToDegrees(atan2f(forward.x(), forward.z()));
        controller.pitch = radiansToDegrees(clamp(asinf(forward.y()), controller.minPitch, controller.maxPitch));
    }

    void FreeFlyControllerSystem::updateTranslation(SystemData const& data, FreeFlyController const& controller, Transform& transform)
    {
        float speed = data.deltaTime * controller.speed;

        if (auto sprintState = m_pInput->getInputState(m_actionMapHandle, "sprint"_sid); sprintState.has_value() && sprintState.value().held)
        {
            speed *= controller.sprint;
        }

        if (auto forwardState = m_pInput->getInputState(m_actionMapHandle, "forward"_sid); forwardState.has_value() && forwardState.value().held)
        {
            transform.translate(transform.forward() * speed);
        }

        if (auto backwardState = m_pInput->getInputState(m_actionMapHandle, "backward"_sid); backwardState.has_value() && backwardState.value().held)
        {
            transform.translate(-transform.forward() * speed);
        }

        if (auto leftState = m_pInput->getInputState(m_actionMapHandle, "left"_sid); leftState.has_value() && leftState.value().held)
        {
            transform.translate(-transform.right() * speed);
        }

        if (auto rightState = m_pInput->getInputState(m_actionMapHandle, "right"_sid); rightState.has_value() && rightState.value().held)
        {
            transform.translate(transform.right() * speed);
        }

        if (auto upState = m_pInput->getInputState(m_actionMapHandle, "up"_sid); upState.has_value() && upState.value().held)
        {
            transform.translate(transform.up() * speed);
        }

        if (auto downState = m_pInput->getInputState(m_actionMapHandle, "down"_sid); downState.has_value() && downState.value().held)
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

        controller.yaw += mouseDeltaState->xyDelta.x() * controller.horizontalSensitivity;
        controller.pitch += (mouseDeltaState->xyDelta.y() * controller.verticalSensitivity) * (controller.invertY ? -1.0f : 1.0f);
        controller.pitch = clamp(controller.pitch, controller.minPitch, controller.maxPitch);

        transform.setRotation(quat::fromEuler(controller.pitch, controller.yaw, 0.0f));
    }
}