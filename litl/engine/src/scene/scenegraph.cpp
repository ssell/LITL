#include <stack>
#include <unordered_map>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-engine/scene/scenegraph.hpp"
#include "litl-core/containers/pagedVector.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    // -------------------------------------------------------------------------------------
    // SceneGraph::Impl
    // -------------------------------------------------------------------------------------

    struct SceneGraph::Impl
    {
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
        std::vector<Entity> nodeToEntity;

        /// <summary>
        /// The scene node index of the parent of the scene node. If no parent, then will be equal to Constants::null_index32.
        /// This represents "child -> parent"
        /// </summary>
        std::vector<uint32_t> nodeParent;

        /// <summary>
        /// This represents "parent -> children"
        /// </summary>
        std::unordered_map<uint32_t, std::vector<uint32_t>> childNodes;

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
        std::vector<NodeState> nodeOccupied;

        // ---------------------------------------------------------------------------------
        // Flattened Scene Tree
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// Topologically sorted and flattened scene tree.
        /// </summary>
        std::vector<uint32_t> sortedNodes;

        // ---------------------------------------------------------------------------------
        // Other
        // ---------------------------------------------------------------------------------

        uint32_t activeCount{ 0 };

        bool isDirty{ true };
        
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
            grow(count, nodeOccupied, NodeState::Vacant);
        }

        [[nodiscard]] uint32_t size() const noexcept
        {
            return static_cast<uint32_t>(nodeToEntity.size());
        }

        [[nodiscard]] bool isPresent(Entity entity) const noexcept
        {
            return 
                !entity.isNull() && 
                (size() > entity.index) && 
                (nodeToEntity[entity.index] == entity) && 
                (nodeOccupied[entity.index] == NodeState::Present);
        }

        [[nodiscard]] Entity getParent(Entity entity) const noexcept
        {
            if (isPresent(entity))
            {
                uint32_t parentNodeIndex = nodeParent[entity.index];

                if (parentNodeIndex != Constants::uint32_null_index)
                {
                    return nodeToEntity[parentNodeIndex];
                }
            }

            return Entity::null();
        }

        void onTrackEntity(Entity entity, Transform const& transform)
        {
            ensureFit(entity.index);
            LITL_ASSERT_MSG(nodeOccupied[entity.index] == NodeState::Vacant, "Attempting to track entity index that is already tracked.", );

            Entity parent = transform.parent.get();

            if (!parent.isNull())
            {
                assert(isPresent(parent) == true);
                childNodes[parent.index].push_back(entity.index);
            }

            updateEntity(entity.index, entity, parent, 0, Constants::uint32_null_index);
            activeCount++;
            isDirty = true;
        }

        void onSetParent(Entity child, Entity parent)
        {
            ensureFit(child.index);
            ensureFit(parent.index);

            LITL_ASSERT_MSG(!child.isNull(), "Attempting to set parent of null child.", );
            LITL_ASSERT_MSG(parent.isNull() || (nodeOccupied[parent.index] == NodeState::Present), "Attempting to set parent to non-null entity that is not tracked.", );

            // remove from parents children
            uint32_t prevParent = nodeParent[child.index];

            if (prevParent != Constants::uint32_null_index)
            {
                auto& siblings = childNodes[prevParent];
                std::erase(siblings, child.index);
            }

            // add to new parent
            if (!parent.isNull())
            {
                childNodes[parent.index].push_back(child.index);
            }

            updateEntity(child.index, child, parent, 0, nodeGpuIndex[child.index]);
            isDirty = true;
        }

        void onUntrackEntity(Entity entity, SceneGraphUntrackBehavior behavior)
        {
            ensureFit(entity.index);
            LITL_ASSERT_MSG(nodeOccupied[entity.index] == NodeState::Present, "Attempting to untrack entity that is not tracked.", );

            // Handle child nodes
            if (childNodes.contains(entity.index))
            {
                if (behavior == SceneGraphUntrackBehavior::Cascade)
                {
                    untrackChildrenOf(entity);
                }
                else // behavior == SceneGraphUntrackBehavior::Orphan
                {
                    orphanChildrenOf(entity);
                }
            }

            // Remove from parent
            uint32_t parentIndex = nodeParent[entity.index];

            if (parentIndex != Constants::uint32_null_index)
            {
                auto& siblings = childNodes[parentIndex];
                std::erase(siblings, entity.index);
            }


            updateEntity(entity.index, Entity::null(), Entity::null(), 0, Constants::uint32_null_index);
            activeCount--;
            isDirty = true;
        }

        void rebuildTree()
        {
            if (!isDirty)
            {
                return;
            }

            isDirty = false;

            // Re-sort the tree using DFS preorder
            sortedNodes.clear();
            std::stack<uint32_t> frontier;

            for (uint32_t i = 0; i < size(); ++i)
            {
                if ((nodeOccupied[i] == NodeState::Present) && (nodeParent[i] == Constants::uint32_null_index))
                {
                    nodeDepth[i] = 0;
                    frontier.push(i);
                }
            }

            while (!frontier.empty())
            {
                uint32_t nodeIndex = frontier.top(); frontier.pop();
                sortedNodes.push_back(nodeIndex);

                auto iter = childNodes.find(nodeIndex);

                if (iter != childNodes.end())
                {
                    for (auto childIndex : iter->second)
                    {
                        nodeDepth[childIndex] = nodeDepth[nodeIndex] + 1;
                        frontier.push(childIndex);
                    }
                }
            }

            LITL_ASSERT_MSG(sortedNodes.size() == activeCount, "Sorted tree size does not equal expected node count.", );
        }

        std::vector<Entity> getChildren(Entity entity) const noexcept
        {
            std::vector<Entity> children;

            if (isPresent(entity))
            {
                auto find = childNodes.find(entity.index);

                if (find != childNodes.end())
                {
                    for (auto childIndex : find->second)
                    {
                        children.push_back(nodeToEntity[childIndex]);
                    }
                }
            }

            return children;
        }

    private:

        template<typename T>
        void grow(uint32_t count, std::vector<T>& v, T defaultValue)
        {
            if (v.size() < count)
            {
                v.resize(count, defaultValue);
            }
        }

        void updateEntity(uint32_t index, Entity entity, Entity parent, uint32_t depth, uint32_t gpuIndex)
        {
            nodeToEntity[index] = entity;
            nodeParent[index] = parent.index;
            nodeDepth[index] = depth;
            nodeGpuIndex[index] = gpuIndex;
            nodeOccupied[index] = (entity.isNull() ? NodeState::Vacant : NodeState::Present);
        }

        void untrackChildrenOf(Entity entity)
        {
            std::vector<uint32_t> descendents;
            std::stack<uint32_t> frontier;

            auto find = childNodes.find(entity.index);

            if (find == childNodes.end())
            {
                return;
            }

            for (auto childIndex : find->second)
            {
                frontier.push(childIndex);
            }

            while (!frontier.empty())
            {
                uint32_t nodeIndex = frontier.top(); frontier.pop();
                descendents.push_back(nodeIndex);

                find = childNodes.find(nodeIndex);

                if (find != childNodes.end())
                {
                    for (auto grandchildIndex : find->second)
                    {
                        frontier.push(grandchildIndex);
                    }
                }
            }

            for (auto nodeIndex : descendents)
            {
                childNodes.erase(nodeIndex);
                updateEntity(nodeIndex, Entity::null(), Entity::null(), 0, Constants::uint32_null_index);
                activeCount--;
            }

            childNodes.erase(entity.index);
        }

        void orphanChildrenOf(Entity entity)
        {
            for (auto childIndex : childNodes[entity.index])
            {
                nodeParent[childIndex] = Constants::uint32_null_index;
            }

            childNodes.erase(entity.index);
        }
    };

    // -------------------------------------------------------------------------------------
    // SceneGraph
    // -------------------------------------------------------------------------------------

    SceneGraph::SceneGraph()
    {
        m_impl->ensureFit(1024);
    }

    SceneGraph::~SceneGraph()
    {

    }

    void SceneGraph::track(Entity entity, Transform const& transform, SceneGraphAccessKey)
    {
        m_impl->onTrackEntity(entity, transform);
    }

    void SceneGraph::setParent(Entity child, Entity parent, SceneGraphAccessKey)
    {
        m_impl->onSetParent(child, parent);
    }

    void SceneGraph::untrackEntity(Entity entity, SceneGraphUntrackBehavior behavior, SceneGraphAccessKey)
    {
        m_impl->onUntrackEntity(entity, behavior);
    }

    void SceneGraph::update(SceneGraphAccessKey)
    {
        m_impl->rebuildTree();
    }

    Entity SceneGraph::getParent(Entity entity) const noexcept
    {
        return m_impl->getParent(entity);
    }

    std::vector<Entity> SceneGraph::getChildren(Entity entity) const noexcept
    {
        return m_impl->getChildren(entity);
    }

    uint32_t SceneGraph::getGpuBufferIndex(Entity entity) const noexcept
    {
        if (m_impl->isPresent(entity))
        {
            return m_impl->nodeGpuIndex[entity.index];
        }

        return Constants::uint32_null_index;
    }

    uint32_t SceneGraph::size() const noexcept
    {
        return m_impl->activeCount;
    }

    bool SceneGraph::isPresent(Entity entity) const noexcept
    {
        return m_impl->isPresent(entity);
    }
}