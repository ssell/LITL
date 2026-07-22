#ifndef LITL_ENGINE_SCENE_GRAPH_H__
#define LITL_ENGINE_SCENE_GRAPH_H__

#include <unordered_map>
#include <vector>

#include "litl-core/math.hpp"
#include "litl-ecs/entity/entity.hpp"
#include "litl-engine/ecs/components/transform.hpp"

namespace litl
{
    class Scene;

    /// <summary>
    /// The scene graph has three main responsibilities:
    /// 
    ///   1. Providing an iterable structure for entity hierarchy (parent -> child)
    ///   2. Providing random access to determine entity direct relatives (entity -> parent, entity -> children)
    ///   3. Maintaining the entity index into the World Matrix buffer.
    /// 
    /// Changes should be done in the order of:
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

        SceneGraph() = default;
        ~SceneGraph() = default;

        SceneGraph(SceneGraph const&) = delete;
        SceneGraph& operator=(SceneGraph const&) = delete;

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
        void add(Entity entity, Transform const& transform) noexcept;

        /// <summary>
        /// Updates the scene tree when the specified entity is to be untracked.
        /// Must call update after for the changes to fully take effect.
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        /// <param name="entity"></param>
        void remove(Entity entity) noexcept;

        /// <summary>
        /// Sets the parent entity for the specified entity.
        /// If the parent is set to null, then the parent is removed.
        /// Must call update after for the changes to fully take effect.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        /// <param name="child"></param>
        /// <param name="parent"></param>
        void setParent(Entity child, Entity parent) noexcept;

        /// <summary>
        /// Re-sorts the tree following one or more changes.
        /// Does nothing if no changes have been made.
        /// 
        /// Note: this is a structural/topological change and can only be called by the appropriate internal systems.
        /// </summary>
        void update() noexcept;

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
        /// <returns></returns>
        uint32_t getChildren(Entity entity, std::vector<Entity>& children, bool recursive = false) const noexcept;

        /// <summary>
        /// Retrieves the index into the GPU buffers that store data associated with the specified entity.
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept;

        // ---------------------------------------------------------------------------------
        // Misc
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Returns the number of active entities tracked by the scene graph.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t count() const noexcept;

        /// <summary>
        /// Returns if the entity is present in the scene graph (not null, has a transform, etc.).
        /// </summary>
        /// <param name="entity"></param>
        /// <returns></returns>
        [[nodiscard]] bool isPresent(Entity entity) const noexcept;

    protected:

    private:
        
        /// <summary>
        /// Its easier/cleaner to just grant the owning Scene access than build in multiple getters that are needed for preRender updates.
        /// </summary>
        friend class Scene;

        /// <summary>
        /// Returns the size of the underlying arrays.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] uint32_t storageSize() const noexcept;

        /// <summary>
        /// Grows all arrays so they can hold at least the specified number of items.
        /// As all of the arrays should have (nearly) the same capacity, this is used
        /// to ensure they all grow uniformly.
        /// </summary>
        /// <param name="index"></param>
        void ensureFit(uint32_t index);

        /// <summary>
        /// Updates an entity and all of its properties.
        /// </summary>
        /// <param name="index"></param>
        /// <param name="entity"></param>
        /// <param name="parent"></param>
        /// <param name="depth"></param>
        /// <param name="gpuIndex"></param>
        void updateEntity(uint32_t index, Entity entity, Entity parent, uint32_t depth, uint32_t gpuIndex);

        enum class NodeState : uint8_t
        {
            Vacant = 0,
            Present = 1
        };

        /**
         * The vectors below represent a flattened version of the following structure:
         *
         *     struct SceneNode {
         *         Entity entity;                           // nodeToEntity
         *         uint32_t depth;                          // nodeDepth
         *         uint32_t gpuBufferIndex;                 // nodeGpuBuffer
         *
         *         uint32_t parentIndex;                    // Random access: nodeParent, Iterative access: treeEdgeParent
         *         std::vector<uint32_t> childIndices;      // Random access: none, Iterative access: treeEdgeChild
         *     };
         *
         *     std::vector<SceneNode> sceneNodes;
         *     std::vector<uint32_t> entityToSceneNode;
         */

         // ---------------------------------------------------------------------------------
         // Scene Node in parallel arrays
         // ---------------------------------------------------------------------------------

         /// <summary>
         /// The entity referenced by a scene node.
         /// </summary>
        std::vector<Entity> m_nodeToEntity;

        /// <summary>
        /// The scene node index of the parent of the scene node. If no parent, then will be equal to Constants::null_index32.
        /// This represents "child -> parent"
        /// </summary>
        std::vector<uint32_t> m_nodeParent;

        /// <summary>
        /// This represents "parent -> children"
        /// </summary>
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_childNodes;

        /// <summary>
        /// The depth of the node in the scene tree. If 0, then the node is a root node with no direct parent.
        /// </summary>
        std::vector<uint32_t> m_nodeDepth;

        /// <summary>
        /// Index into the GPU buffers (world matrix, etc.).
        /// The key is the node index and the value is the gpu/sorted index.
        /// 
        /// This index is not static and may change each frame. This is because
        /// the scene is not static and the number of active entities is in flux.
        /// As the GPU buffers can not have gaps, the indices may change to ensure
        /// the buffers are whole.
        /// 
        /// For example, if we have two top level nodes, each with two children:
        /// 
        ///   0 -> 1
        ///     -> 4
        /// 
        ///   2 -> 3
        ///     -> 5
        /// 
        /// The GPU index (and sorted order) would be: [0, 1, 3, 4, 2, 5]
        /// 
        /// So,
        /// 
        ///     Node 0 => GPU / Sorted Index 0
        ///     Node 1 => GPU / Sorted Index 1
        ///     Node 2 => GPU / Sorted Index 3
        ///     Node 3 => GPU / Sorted Index 4
        ///     Node 4 => GPU / Sorted Index 2
        ///     Node 5 => GPU / Sorted Index 5
        /// </summary>
        std::vector<uint32_t> m_nodeGpuIndex;

        /// <summary>
        /// If true, then the node correlates to an entity that is both alive and has a transform component.
        /// </summary>
        std::vector<NodeState> m_nodeOccupied;

        // ---------------------------------------------------------------------------------
        // Flattened Scene Tree
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Topologically sorted and flattened scene tree.
        /// The key is the sorted order and the value is the node index.
        /// 
        /// For example, if we have two top level nodes, each with two children:
        /// 
        ///   0 -> 1
        ///     -> 4
        /// 
        ///   2 -> 3
        ///     -> 5
        /// 
        /// The sorted order would be: [0, 1, 4, 2, 3, 5]
        /// 
        /// So,
        /// 
        ///     Sorted Node 0 => Node Index 0
        ///     Sorted Node 1 => Node Index 1
        ///     Sorted Node 2 => Node Index 4
        ///     Sorted Node 3 => Node Index 2
        ///     Sorted Node 4 => Node Index 3
        ///     Sorted Node 5 => Node Index 5
        /// </summary>
        std::vector<uint32_t> m_sortedNodes;

        // ---------------------------------------------------------------------------------
        // Other
        // ---------------------------------------------------------------------------------

        uint32_t m_activeCount{ 0 };

        bool m_isDirty{ true };
    };
}

#endif