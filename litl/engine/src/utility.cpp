#include "litl-engine/utility.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // Create Main Camera
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates a new camera and assigns it as the main camera.
    /// </summary>
    Camera* createMainCamera(CameraDescriptor const& descriptor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
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
    Camera* createMainCamera(color clearColor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept
    {
        return createMainCamera(CameraDescriptor{ .clearColor = clearColor }, position, target, up, objectPool, sceneView);
    }

    // -------------------------------------------------------------------------------------
    // Create Renderable
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates a new entity and attaches to it the minimal components needed for it to be rendered.
    /// This consists of: Transform, MeshRef, MaterialRef (with a single material), and LocalBounds.
    /// </summary>
    DeferredEntity createRenderable(vec3 position, std::string_view mesh, std::string_view material, EntityCommands& commands, AssetManager& assets) noexcept
    {
        const DeferredEntity entity = commands.createEntity();

        commands.addComponent<PendingMeshInstance>(entity, PendingMeshInstance{ .meshHandle = assets.getMeshHandle(mesh) });
        commands.addComponent<LocalBounds>(entity, LocalBounds{});      // This will be set when the mesh is instantiated

        auto* materialAsset = assets.getMaterial(material);

        if ((materialAsset != nullptr) && (materialAsset->material != nullptr))
        {
            commands.addComponent<MaterialRef>(entity, MaterialRef{
                .handle = materialAsset->materialHandle,
                .slot = materialAsset->material->allocateSlot()
            });
        }

        commands.addComponent<Transform>(entity, Transform::create(position));

        return entity;
    }

    // -------------------------------------------------------------------------------------
    // Get Model Instance
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Initiates loading the specified model (if it is not already in memory) and returns a PendingModelInstance component
    /// to be attached to the entity that will serve as the root for the loaded model. Once in memory, the PendingModelInstance
    /// component is replaced by a ModelInstanceComponent (or FailedModelInstanceComponent on error) and children added to the entity
    /// reflecting the model internal node hierarchy with meshes and materials attached to those children as necessary.
    /// </summary>
    PendingModelInstance createModelInstance(std::string_view resource, AssetManager& assetManager) noexcept
    {
        auto* modelAsset = assetManager.getModel(resource);

        if (modelAsset == nullptr)
        {
            return {};
        }

        return PendingModelInstance
        {
            .modelHandle = modelAsset->selfHandle.modelHandle
        };
    }

    /// <summary>
    /// Variant of createModelInstance that allows the specification of a fallback material to be used if any of the model meshes
    /// do not have a valid material assigned to them.
    /// </summary>
    PendingModelInstance createModelInstance(std::string_view modelResource, std::string_view fallbackMaterialResource, AssetManager& assetManager) noexcept
    {
        auto* modelAsset = assetManager.getModel(modelResource);

        if (modelAsset == nullptr)
        {
            return {};
        }

        PendingModelInstance pendingModel{ .modelHandle = modelAsset->selfHandle.modelHandle };

        if (pendingModel.modelHandle.isValid())
        {
            auto* material = assetManager.getMaterial(fallbackMaterialResource);

            if ((material != nullptr) && (material->material != nullptr))
            {
                pendingModel.fallbackMaterialHandle = material->materialHandle;
                pendingModel.fallbackMaterialSlot = material->material->allocateSlot();
            }
        }

        return pendingModel;
    }
}