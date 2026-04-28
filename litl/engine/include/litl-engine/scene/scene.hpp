#ifndef LITL_ENGINE_SCENE_H__
#define LITL_ENGINE_SCENE_H__

#include "litl-core/impl.hpp"
#include "litl-core/math/bounds.hpp"

#include "litl-engine/scene/sceneAccessKey.hpp"
#include "litl-engine/scene/sceneConfig.hpp"
#include "litl-engine/ecs/components/transform.hpp"

namespace litl
{
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

        Scene(SceneConfig const& config);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

        /// <summary>
        /// Adds the entity to the scene.
        /// Must call sync for all changes to take effect.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        /// <param name="bounds"></param>
        void add(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept;

        /// <summary>
        /// Removes the entity from the scene.
        /// Must call sync for all changes to take effect.
        /// </summary>
        /// <param name="entity"></param>
        void remove(Entity entity) noexcept;

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
        /// <returns></returns>
        [[nodiscard]] std::vector<Entity> getChildren(Entity entity) const noexcept;

        /// <summary>
        /// Returns the index into the GPU buffers which the entity occupies.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept;

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
        /// Syncs all hierarchical changes (parent/child) and any newly added or removed entities.
        /// </summary>
        void sync() noexcept;

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 544> m_impl;
    };
}

#endif