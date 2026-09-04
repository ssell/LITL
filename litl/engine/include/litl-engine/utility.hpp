/**
 * A collection of commonly used utility functions such as "createRenderable".
 */

#ifndef LITL_ENGINE_UTILITY_H__
#define LITL_ENGINE_UTILITY_H__

#include "litl-core/math.hpp"
#include "litl-core/stringId.hpp"
#include "litl-engine/assets/assetManager.hpp"
#include "litl-engine/ecs/common.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/scene/sceneView.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // Create Main Camera
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates a new camera and assigns it as the main camera.
    /// </summary>
    inline Camera* createMainCamera(CameraDescriptor const& descriptor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        auto cameraHandle = objectPool.createCamera(descriptor);
        auto* camera = objectPool.getCamera(cameraHandle);

        sceneView.setMainCamera(cameraHandle);
        camera->setWorldPosition(position);
        camera->lookAt(target, up);

        return camera;
    }

    /// <summary>
    /// Creates a new camera and assigns it as the main camera.
    /// </summary>
    inline Camera* createMainCamera(color clearColor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        return createMainCamera(CameraDescriptor{ .clearColor = clearColor }, position, target, up, objectPool, sceneView);
    }

    // -------------------------------------------------------------------------------------
    // Create Renderable
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates a new entity and attaches to it the minimal components needed for it to be rendered.
    /// This consists of: Transform, MeshRef, MaterialRef, and LocalBounds.
    /// </summary>
    inline DeferredEntity createRenderable(vec3 position, StringId mesh, StringId material, EntityCommands& commands, AssetManager& assets) noexcept
    {
        const DeferredEntity entity = commands.createEntity();
        auto* meshAsset = assets.getMesh(mesh);

        if ((meshAsset != nullptr) && (meshAsset->mesh != nullptr))
        {
            commands.addComponent<MeshRef>(entity, MeshRef{ .handle = meshAsset->handle });
            commands.addComponent<LocalBounds>(entity, LocalBounds{ .bounds = meshAsset->mesh->getBounds() });
        }

        commands.addComponent<Transform>(entity, Transform::create(position));
        commands.addComponent<MaterialRef>(entity, assets.getMaterialRef(material));

        return entity;
    }

    /// <summary>
    /// Creates a new entity and attaches to it the minimal components needed for it to be rendered.
    /// This consists of: Transform, MeshRef, MaterialRef, and LocalBounds.
    /// </summary>
    inline DeferredEntity createRenderable(vec3 position, std::string_view mesh, std::string_view material, EntityCommands& commands, AssetManager& assets) noexcept
    {
        return createRenderable(position, StringId(mesh), StringId(material), commands, assets);
    }
}

#endif