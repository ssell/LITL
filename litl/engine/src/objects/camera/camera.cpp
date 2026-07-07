#include "litl-engine/objects/camera/camera.hpp"

namespace litl
{
    void Camera::create(CameraDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
    }

    void Camera::destroy() noexcept
    {
        // ... todo? ...
    }

    CameraDescriptor const& Camera::getDescriptor() const noexcept
    {
        return m_descriptor;
    }
}