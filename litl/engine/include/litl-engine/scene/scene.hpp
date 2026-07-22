#ifndef LITL_ENGINE_SCENE_H__
#define LITL_ENGINE_SCENE_H__

#include <variant>

#include "litl-core/authority.hpp"
#include "litl-core/math/bounds.hpp"
#include "litl-engine/scene/sceneConfig.hpp"
#include "litl-engine/ecs/components/transform.hpp"
#include "litl-engine/scene/sceneGraph.hpp"
#include "litl-engine/scene/sceneTransforms.hpp"
#include "litl-engine/scene/sceneCameras.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"
#include "litl-engine/scene/partition/nullPartition.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    class SceneManager;
    class Renderer;
    class ObjectPool;
    class World;

    /// <summary>
    /// Responsible for tracking entities and their relationships to each other.
    /// 
    /// The Scene itself should generally not be used directly. Instead either a SceneView
    /// for read/query operations or EntityCommands for structural changes should be used.
    /// This is because Scene, and its underlying classes such as SceneGraph, ScenePartition, etc.
    /// are not thread-safe by default.
    /// </summary>
    class Scene
    {
    public:

        Scene(SceneConfig const& config, Renderer const* renderer, ObjectPool* objectPool);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        /// <summary>
        /// Adds the entity to the scene.
        /// 
        /// As no bounds is provided, it will be tracked in the scene partition
        /// using an unit cube AABB.
        /// 
        /// Must call sync for all changes to take effect.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        void track(Entity entity, Transform const& transform) noexcept;

        /// <summary>
        /// Adds the entity to the scene.
        /// Must call sync for all changes to take effect.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        /// <param name="bounds"></param>
        void track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept;

        /// <summary>
        /// Removes the entity from the scene.
        /// Must call sync for all changes to take effect.
        /// </summary>
        /// <param name="entity"></param>
        void untrack(Entity entity) noexcept;

        /// <summary>
        /// Updates the local bounds of the entity in the scene.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="bounds"></param>
        void update(Entity entity, bounds::AABB bounds) noexcept;

        /// <summary>
        /// Returns true if the entity is present in the scene.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] bool isPresent(Entity entity) const noexcept;

        /// <summary>
        /// Returns the parent of the entity. If it has no parent, returns Entity::null().
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] Entity getParent(Entity entity) const noexcept;

        /// <summary>
        /// Sets the parent of the specified entity.
        /// If the parent is null this remove the entity from its parent.
        /// </summary>
        /// <param name="child"></param>
        /// <param name="parent"></param>
        void setParent(Entity child, Entity parent) noexcept;

        /// <summary>
        /// Returns all entities that are children of the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="recursive">If false, then only direct descendants (immediate children) will be returned.</param>
        /// <returns></returns>
        [[nodiscard]] std::vector<Entity> getChildren(Entity entity, bool recursive = false) const noexcept;

        /// <summary>
        /// Appends all children of the specified entity to the provided vector.
        /// Returns the number of children added.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="children"></param>
        /// <param name="recursive">If false, then only direct descendants (immediate children) will be returned.</param>
        uint32_t getChildren(Entity entity, std::vector<Entity>& children, bool recursive = false) const noexcept;

        /// <summary>
        /// Returns the index into the GPU buffers which the entity occupies.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept;

        /// <summary>
        /// Retrieves the previously calculated world matrix for the specified entity.
        /// World matrices are calculated once per frame immediately prior the PreRender ECS grouping.
        /// As such the matrix may be up to one frame stale.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] mat4 getWorldMatrix(Entity entity) const noexcept;

        /// <summary>
        /// Returns a read-only span of all entity world matrices.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] std::span<mat4 const> getWorldMatrices() const noexcept;

        /// <summary>
        /// Invoked once per-frame immediately before the PreRender ECS group.
        /// Updates scene hierarchy, world transforms, and spatial partition.
        /// </summary>
        /// <param name="authority"></param>
        void onPreRender(Authority<SceneManager> authority, World& world) noexcept;

        /// <summary>
        /// Returns all entities that are within or intersect the specified AABB.
        /// </summary>
        /// <param name="aabb"></param>
        /// <param name="entities"></param>
        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept;
        
        /// <summary>
        /// Returns all entities that are within or intersect the specified Sphere.
        /// </summary>
        /// <param name="sphere"></param>
        /// <param name="entities"></param>
        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept;

        /// <summary>
        /// Returns all entities that are within or intersect the specified Frustum.
        /// </summary>
        /// <param name="frustum"></param>
        /// <param name="entities"></param>
        void query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept;

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
        CameraHandle getMainCameraHandle() const noexcept;

        /// <summary>
        /// Returns the camera that was last designated as the main camera.
        /// If no main camera was set, or it has been destroyed, this may return null.
        /// </summary>
        /// <returns></returns>
        Camera* getMainCamera() const noexcept;

        /// <summary>
        /// Returns all cameras. They are sorted according to their process order value.
        /// </summary>
        /// <returns></returns>
        std::span<Camera*> getCameras() noexcept;

    protected:

    private:

        using ScenePartitionVariant = std::variant<
            NullPartition,
            UniformGridPartition
            /* add future partition strategies here */
        >;

        Renderer const* m_pRenderer{ nullptr };
        SceneTransforms m_transforms;
        SceneGraph m_graph;
        ScenePartitionVariant m_partition;
        SceneCameras m_cameras;
    };
}

#endif