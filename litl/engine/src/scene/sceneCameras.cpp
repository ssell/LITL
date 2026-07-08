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
        if (m_mainCameraHandle == handle)
        {
            return;
        }

        for (auto* camera : m_cameras)
        {
            camera->setAsMainCamera({}, false);
        }

        m_mainCameraHandle = handle;
        auto* mainCamera = m_pObjectPool->getCamera(handle);

        if (mainCamera != nullptr)
        {
            mainCamera->setAsMainCamera({}, true);
        }
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