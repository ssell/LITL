#include "litl-engine/scene/sceneCameras.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/camera/camera.hpp"

namespace litl
{
    void SceneCameras::setup(ObjectPool& objectPool) noexcept
    {
        m_pObjectPool = &objectPool;
    }

    void SceneCameras::setMainCamera(CameraHandle handle) noexcept
    {
        m_mainCameraHandle = handle;
    }

    CameraHandle SceneCameras::getMainCameraHandle() const noexcept
    {
        return m_mainCameraHandle;
    }

    Camera* SceneCameras::getMainCamera() const noexcept
    {
        auto handle = getMainCameraHandle();
        return m_pObjectPool->getCamera(handle);
    }

    std::span<Camera*> SceneCameras::getCameras() noexcept
    {
        return m_cameras;
    }
}