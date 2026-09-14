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
    Camera* createMainCamera(CameraDescriptor const& descriptor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept;

    /// <summary>
    /// Creates a new camera and assigns it as the main camera.
    /// </summary>
    Camera* createMainCamera(color clearColor, vec3 position, vec3 target, vec3 up, ObjectPool& objectPool, SceneView& sceneView) noexcept;

    // -------------------------------------------------------------------------------------
    // Create Renderable
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Creates a new entity and attaches to it the minimal components needed for it to be rendered.
    /// This consists of: Transform, MeshRef, MaterialRef (with a single material), and LocalBounds.
    /// </summary>
    DeferredEntity createRenderable(vec3 position, std::string_view mesh, std::string_view material, EntityCommands& commands, AssetManager& assets) noexcept;

    // -------------------------------------------------------------------------------------
    // Create Model Instance
    // -------------------------------------------------------------------------------------

    /// <summary>
    /// Initiates loading the specified model (if it is not already in memory) and returns a PendingModelInstance component
    /// to be attached to the entity that will serve as the root for the loaded model. Once in memory, the PendingModelInstance
    /// component is replaced by a ModelInstanceComponent (or FailedModelInstanceComponent on error) and children added to the entity
    /// reflecting the model internal node hierarchy with meshes and materials attached to those children as necessary.
    /// </summary>
    PendingModelInstance createModelInstance(std::string_view resource, AssetManager& assetManager) noexcept;

    /// <summary>
    /// Variant of createModelInstance that allows the specification of a fallback material to be used if any of the model meshes
    /// do not have a valid material assigned to them.
    /// </summary>
    PendingModelInstance createModelInstance(std::string_view modelResource, std::string_view fallbackMaterialResource, AssetManager& assetManager) noexcept;
}

#endif