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

    /// <summary>
    /// The scene graph has three main responsibilities:
    /// 
    ///   1. Providing an iterable structure for entity hierarchy (parent -> child)
    ///   2. Providing random access to determine entity direct relatives (entity -> parent, entity -> children)
    ///   3. Maintaining the entity index into the World Matrix buffer.
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
        void untrackEntity(Entity entity, SceneGraphAccessKey);

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
        ImplPtr<Impl, 280> m_impl;
    };
}

#endif