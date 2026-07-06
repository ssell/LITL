#ifndef LITL_ENGINE_SCENE_CULLING_H__
#define LITL_ENGINE_SCENE_CULLING_H__

#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/camera.hpp"

namespace litl
{
    class SceneCulling
    {
    public:

        SceneCulling() = default;
        SceneCulling(SceneCulling const&) = delete;
        SceneCulling& operator=(SceneCulling const&) = delete;

        /// <summary>
        /// Registers that the specified camera is rendering this frame.
        /// </summary>
        void registerCameraForFrame(Entity entity, Camera camera) noexcept;

    private:


    };
}

#endif