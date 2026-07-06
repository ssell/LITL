#ifndef LITL_ENGINE_SCENE_DATA_H__
#define LITL_ENGINE_SCENE_DATA_H__

#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/camera.hpp"

namespace litl
{
    /// <summary>
    /// Stores misc data (usually transient) that has no better home.
    /// </summary>
    class SceneData
    {
        struct TrackedCameraData
        {
            Entity entity{};
            CameraData data{};
        };

    public:

        SceneData();
        SceneData(SceneData const&) = delete;
        SceneData& operator=(SceneData const&) = delete;

        void trackCameraForFrame(Entity entity) noexcept;
        CameraData& getCameraData(Entity entity) noexcept;
        CameraData const& getCameraData(Entity entity) const noexcept;

    private:

        std::vector<TrackedCameraData> m_cameraDatums;
    };
}

#endif