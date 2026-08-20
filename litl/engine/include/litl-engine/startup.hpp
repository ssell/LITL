#ifndef LITL_ENGINE_STARTUP_H__
#define LITL_ENGINE_STARTUP_H__

// Collection of headers commonly needed for setting up an application using LITL
#include "litl-engine/engine.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/scene/sceneManager.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-core/file.hpp"

namespace litl
{
    inline void createMainCamera(CameraDescriptor const& descriptor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        auto cameraHandle = objectPool.createCamera(descriptor);
        auto* camera = objectPool.getCamera(cameraHandle);

        sceneView.setMainCamera(cameraHandle);
        camera->setWorldPosition(position);
        camera->lookAt(target, up);
    }
}

#endif