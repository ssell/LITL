#include "litl-engine/scene/scenegraph.hpp"
#include "litl-core/containers/pagedVector.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    struct SceneGraph::Impl
    {
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
        std::vector<Entity> nodeToEntity;

        /// <summary>
        /// The scene node index of the parent of the scene node. If no parent, then will be equal to Constants::null_index32.
        /// </summary>
        std::vector<uint32_t> nodeParent;

        /// <summary>
        /// The depth of the node in the scene tree. If 0, then the node is a root node with no direct parent.
        /// </summary>
        std::vector<uint32_t> nodeDepth;

        /// <summary>
        /// Index into the GPU buffers (world matrix, etc.).
        /// </summary>
        std::vector<uint32_t> nodeGpuIndex;

        /// <summary>
        /// If true, then the node correlates to an entity that is both alive and has a transform component.
        /// </summary>
        std::vector<bool> nodeOccupied;

        // ---------------------------------------------------------------------------------
        // Flattened Scene Tree
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// The key is the edge index. The value is the scene node index of the parent scene node of the edge connecting Parent -> Child.
        /// This differs from nodeParent as that takes the node index (and not the edge index) as the accessor.
        /// </summary>
        std::vector<uint32_t> treeEdgeParent;

        /// <summary>
        /// The key is the edge index. The value is the scene node index of the child scene node of the edge connecting Parent -> Child.
        /// </summary>
        std::vector<uint32_t> treeEdgeChild;

        /// <summary>
        /// Topologically sorted and flattened scene tree.
        /// </summary>
        std::vector<uint32_t> sortedNodes;

        // ---------------------------------------------------------------------------------
        // Other
        // ---------------------------------------------------------------------------------

        bool isDirty{ false };
        
        /// <summary>
        /// Grows all arrays so they can hold at least the specified number of items.
        /// As all of the arrays should have (nearly) the same capacity, this is used
        /// to ensure they all grow uniformly.
        /// </summary>
        /// <param name="count"></param>
        void ensureFit(uint32_t index)
        {
            // These two should be the same value, but just incase ...
            if (index == Constants::uint32_null_index)
            {
                return;
            }

            uint32_t count = index + 1;

            if (size() >= count)
            {
                // Since all arrays grow together, we check just one to see if it can contain the specified index.
                return;
            }

            grow(count, nodeToEntity, Entity::null());
            grow(count, nodeParent, Constants::uint32_null_index);
            grow(count, nodeDepth, 0u);
            grow(count, nodeGpuIndex, Constants::uint32_null_index);
            grow(count, nodeOccupied, false);

            // For edge arrays the worst-case size is N-1 (all nodes nested under a single parent), but grow to N for simplicity.
            grow(count, treeEdgeParent, Constants::uint32_null_index);
            grow(count, treeEdgeChild, Constants::uint32_null_index);
            grow(count, sortedNodes, Constants::uint32_null_index);
        }

        [[nodiscard]] uint32_t size() const noexcept
        {
            return static_cast<uint32_t>(nodeToEntity.size());
        }

        [[nodiscard]] bool isValid(Entity entity) const noexcept
        {
            return !entity.isNull() && (size() > entity.index) && (nodeToEntity[entity.index] == entity) && (nodeOccupied[entity.index] == true);
        }

        [[nodiscard]] Entity getParent(Entity entity) const noexcept
        {
            if (isValid(entity))
            {
                uint32_t parentNodeIndex = nodeParent[entity.index];

                if (parentNodeIndex != Constants::uint32_null_index)
                {
                    return nodeToEntity[parentNodeIndex];
                }
            }

            return Entity::null();
        }

    private:

        template<typename T>
        void grow(uint32_t count, std::vector<T>& v, T defaultValue)
        {
            if (v.size() < count)
            {
                v.reserve(count);
                
                while (v.size() < count)
                {
                    v.push_back(defaultValue);
                }
            }
        }
    };

    SceneGraph::SceneGraph()
    {
        m_impl->ensureFit(1024);
    }

    SceneGraph::~SceneGraph()
    {

    }

    void SceneGraph::track(Entity entity, Transform const& transform, SceneGraphAccessKey)
    {
        m_impl->ensureFit(entity.index);
        m_impl->ensureFit(transform.parent.get().index);

        // If this asserts, then either (a) multiple calls to track or (b) bad cleanup of previous occupant.
        assert(m_impl->nodeOccupied[entity.index] == false);

        EntityId parentId = transform.parent.get().index;

        m_impl->isDirty = true;
        m_impl->nodeToEntity[entity.index] = entity;
        m_impl->nodeParent[entity.index] = parentId;

        // parent itself may not yet be tracked so the remaining entries (depth, edges, etc.) are deferred until the call to restructure is made.
    }

    void SceneGraph::setParent(Entity child, Entity parent, SceneGraphAccessKey)
    {
        assert(!child.isNull());

        m_impl->ensureFit(child.index);
        m_impl->ensureFit(parent.index);

        // If this asserts then the old occupant has not been cleared yet or the new entity has not yet been tracked.
        assert(m_impl->nodeToEntity[child.index] == child);
        assert(m_impl->nodeOccupied[child.index] == true);

        m_impl->isDirty = true;
        m_impl->nodeParent[child.index] = parent.index;

        // parent itself may not yet be tracked so the remaining entries (depth, edges, etc.) are deferred until the call to restructure is made.
    }
    
    void SceneGraph::removeAllChildren(Entity entity, SceneGraphAccessKey)
    {
        // ... todo ...
    }

    void SceneGraph::onEntityDestroyed(Entity entity, SceneGraphAccessKey)
    {
        // ... todo ...
    }

    void SceneGraph::restructure(SceneGraphAccessKey)
    {
        // ... todo ...
    }

    Entity SceneGraph::getParent(Entity entity) const noexcept
    {
        return m_impl->getParent(entity);
    }
}