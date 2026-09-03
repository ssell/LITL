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

    inline Camera* createMainCamera(CameraDescriptor const& descriptor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        auto cameraHandle = objectPool.createCamera(descriptor);
        auto* camera = objectPool.getCamera(cameraHandle);

        sceneView.setMainCamera(cameraHandle);
        camera->setWorldPosition(position);
        camera->lookAt(target, up);

        return camera;
    }

    inline Camera* createMainCamera(color clearColor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        return createMainCamera(CameraDescriptor{ .clearColor = clearColor }, position, target, up, objectPool, sceneView);
    }

    inline DeferredEntity createRenderable(EntityCommands& commands, AssetManager& assets, vec3 position, StringId mesh, StringId material) noexcept
    {
        const DeferredEntity entity = commands.createEntity();
        auto* meshAsset = assets.getMesh(mesh);

        if ((meshAsset != nullptr) && (meshAsset->mesh != nullptr))
        {
            commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshAsset->handle });
            commands.addComponent<LocalBounds>(entity, LocalBounds{ .bounds = meshAsset->mesh->getBounds() });
        }

        commands.addComponent<Transform>(entity, Transform::create(position));
        commands.addComponent<MaterialRef>(entity, assets.getMaterialRef("materials/flat"));

        return entity;
    }

    inline DeferredEntity createRenderable(EntityCommands& commands, AssetManager& assets, vec3 position, std::string_view mesh, std::string_view material) noexcept
    {
        return createRenderable(commands, assets, position, StringId(mesh), StringId(material));
    }
}

#endif