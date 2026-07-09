#ifndef LITL_ENGINE_SCENE_CAMERAS_H__
#define LITL_ENGINE_SCENE_CAMERAS_H__

#include <vector>
#include <span>

#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class ObjectPool;
    class Camera;

    class SceneCameras
    {
    public:

        static constexpr uint32_t MaxSceneCameras = 32u;

        void setup(ObjectPool& objectPool) noexcept;

        /// <summary>
        /// Updates the list of cameras from those stored in the object pool and then sorts them.
        /// </summary>
        void update() noexcept;

        /// <summary>
        /// Sets the specified valid camera handle as the main camera.
        /// The main camera is the one used to render to the primary swapchain render target.
        /// </summary>
        /// <param name="handle"></param>
        void setMainCamera(CameraHandle handle) noexcept;

        /// <summary>
        /// Returns the handle of the camera that was last designated as the main camera.
        /// If no main camera was set, or it has been destroyed, this may return an invalid handle.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] CameraHandle getMainCameraHandle() const noexcept;

        /// <summary>
        /// Returns the camera that was last designated as the main camera.
        /// If no main camera was set, or it has been destroyed, this may return null.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] Camera* getMainCamera() const noexcept;

        /// <summary>
        /// Returns all cameras. They are sorted according to their process order value.
        /// </summary>
        /// <returns></returns>
        std::span<Camera*> getCameras() noexcept;

    private:

        ObjectPool* m_pObjectPool = nullptr;
        CameraHandle m_mainCameraHandle{};

        std::vector<CameraHandle> m_cameraHandles;
        std::vector<Camera*> m_cameras;
    };
}

#endif