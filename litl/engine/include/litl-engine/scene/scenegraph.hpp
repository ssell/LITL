#ifndef LITL_ENGINE_SCENE_GRAPH_H__
#define LITL_ENGINE_SCENE_GRAPH_H__

#include "litl-core/impl.hpp"
#include "litl-core/math.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/transform.hpp"

namespace litl
{
    struct SceneGraphAccessKey
    {
    private:

        SceneGraphAccessKey() = default;
        // friend class ...
    };

    enum class SceneGraphUntrackBehavior : uint32_t
    {
        /// <summary>
        /// When an entity is untracked, all children of it are also untracked.
        /// </summary>
        Cascade = 0,

        /// <summary>
        /// When an entity is untracked, all immediate children are orphaned with
        /// their parent being set to null (the world) and they become root nodes.
        /// </summary>
        Orphan = 1
    };

    /// <summary>
    /// The scene graph has three main responsibilities:
    /// 
    ///   1. Providing an iterable structure for entity hierarchy (parent -> child)
    ///   2. Providing random access to determine entity direct relatives (entity -> parent, entity -> children)
    ///   3. Maintaining the entity index into the World Matrix buffer.
    /// 
    /// Structural changes (track, untrack, set parent, update) can only be done via those
    /// classes authorized to do so via the SceneGraphAccessKey. Structural changes should
    /// be buffered and performed at sync points. Changes should be done in the order of:
    /// 
    ///   1. Untrack
    ///   2. Track
    ///   3. Set Parent
    /// 
    /// As is done with other systems, such as entity structural changes, scene graph changes
    /// should be deduped and also cancel out opposing changes. For example, a call to untrack
    /// an entity should cancel out any call to track it, set its parent, or set it as a parent.
    /// </summary>
    class SceneGraph
    {
    public:

        SceneGraph();
        ~SceneGraph();

        // ---------------------------------------------------------------------------------
        // Structural / Topological Changes
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Tracks the entity in the scene graph.
        /// Must call update after for the changes to fully take effect.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="transform"></param>
        void track(Entity entity, Transform const& transform, SceneGraphAccessKey);

        /// <summary>
        /// Sets the parent entity for the specified entity.
        /// If the parent is set to null, then the parent is removed.
        /// Must call update after for the changes to fully take effect.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        /// <param name="child"></param>
        /// <param name="parent"></param>
        void setParent(Entity child, Entity parent, SceneGraphAccessKey);

        /// <summary>
        /// Updates the scene tree when the specified entity is to be untracked.
        /// Must call update after for the changes to fully take effect.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        /// <param name="entity"></param>
        /// <param name="behavior"></param
        void untrackEntity(Entity entity, SceneGraphUntrackBehavior behavior, SceneGraphAccessKey);

        /// <summary>
        /// Re-sorts the tree following one or more changes.
        /// Does nothing if no changes have been made.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        void update(SceneGraphAccessKey);

        // ---------------------------------------------------------------------------------
        // Data Accessors
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Retrieves the Entity which is the parent of the specified Entity.
        /// If there is no parent (Entity is a root node), then Entity::null() is returned.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] Entity getParent(Entity entity) const noexcept;

        /// <summary>
        /// Retrieves all child Entities of the specified Entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] std::vector<Entity> getChildren(Entity entity) const noexcept;

        /// <summary>
        /// Retrieves the index into the GPU buffers that store data associated with the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept;

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 296> m_impl;
    };
}

#endif