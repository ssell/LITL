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

        void setMainCamera(CameraHandle handle) noexcept;
        [[nodiscard]] CameraHandle getMainCameraHandle() const noexcept;
        [[nodiscard]] Camera* getMainCamera() const noexcept;
        std::span<Camera*> getCameras() noexcept;

    private:

        ObjectPool* m_pObjectPool = nullptr;
        CameraHandle m_mainCameraHandle{};

        std::vector<Camera*> m_cameras;
    };
}

#endif