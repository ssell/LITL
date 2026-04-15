#include <stack>
#include <unordered_map>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-engine/scene/scenegraph.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    namespace
    {
        template<typename T>
        void grow(uint32_t count, std::vector<T>& v, T defaultValue)
        {
            if (v.size() < count)
            {
                v.resize(count, defaultValue);
            }
        }
    }

    void SceneGraph::add(Entity entity, Transform const& transform)
    {
        ensureFit(entity.index);
        LITL_ASSERT_MSG(m_nodeOccupied[entity.index] == NodeState::Vacant, "Attempting to track entity index that is already tracked.", );

        Entity parent = transform.parent.get();

        if (!parent.isNull())
        {
            LITL_ASSERT_MSG(entity != parent, "Attempting to set child as parent of itself.", );
            LITL_ASSERT_MSG(isPresent(parent) == true, "Attempting to set invalid parent of child.", );
            m_childNodes[parent.index].push_back(entity.index);
        }

        updateEntity(entity.index, entity, parent, 0, Constants::uint32_null_index);
        m_activeCount++;
        m_isDirty = true;
    }

    void SceneGraph::remove(Entity entity)
    {
        ensureFit(entity.index);
        LITL_ASSERT_MSG(m_nodeOccupied[entity.index] == NodeState::Present, "Attempting to untrack entity that is not tracked.", );

        // Handle child nodes
        if (m_childNodes.contains(entity.index))
        {
            removeChildrenOf(entity);
        }

        // Remove from parent
        uint32_t parentIndex = m_nodeParent[entity.index];

        if (parentIndex != Constants::uint32_null_index)
        {
            auto& siblings = m_childNodes[parentIndex];
            std::erase(siblings, entity.index);
        }

        updateEntity(entity.index, Entity::null(), Entity::null(), 0, Constants::uint32_null_index);
        m_activeCount--;
        m_isDirty = true;
    }

    void SceneGraph::setParent(Entity child, Entity parent)
    {
        ensureFit(child.index);
        ensureFit(parent.index);

        LITL_ASSERT_MSG(!child.isNull(), "Attempting to set parent of null child.", );
        LITL_ASSERT_MSG(child != parent, "Attempting to set child as parent of itself.", );
        LITL_ASSERT_MSG(parent.isNull() || isPresent(parent), "Attempting to set invalid parent of child.", );

        // remove from parents children
        uint32_t prevParent = m_nodeParent[child.index];

        if (prevParent != Constants::uint32_null_index)
        {
            auto& siblings = m_childNodes[prevParent];
            std::erase(siblings, child.index);
        }

        // add to new parent
        if (!parent.isNull())
        {
            m_childNodes[parent.index].push_back(child.index);
        }

        updateEntity(child.index, child, parent, 0, m_nodeGpuIndex[child.index]);
        m_isDirty = true;
    }

    void SceneGraph::update()
    {
        if (!m_isDirty)
        {
            return;
        }

        m_isDirty = false;

        // Re-sort the tree using DFS preorder
        m_sortedNodes.clear();
        std::stack<uint32_t> frontier;

        for (uint32_t i = 0; i < storageSize(); ++i)
        {
            if ((m_nodeOccupied[i] == NodeState::Present) && (m_nodeParent[i] == Constants::uint32_null_index))
            {
                m_nodeDepth[i] = 0;
                frontier.push(i);
            }
        }

        while (!frontier.empty())
        {
            uint32_t nodeIndex = frontier.top(); frontier.pop();
            m_sortedNodes.push_back(nodeIndex);

            auto iter = m_childNodes.find(nodeIndex);

            if (iter != m_childNodes.end())
            {
                for (auto childIndex : iter->second)
                {
                    m_nodeDepth[childIndex] = m_nodeDepth[nodeIndex] + 1;
                    frontier.push(childIndex);
                }
            }
        }

        LITL_ASSERT_MSG(m_sortedNodes.size() == m_activeCount, "Sorted tree size does not equal expected node count.", );
    }

    Entity SceneGraph::getParent(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG(isPresent(entity), "Attempting to retrieve parent of untracked entity.", Entity::null());

        uint32_t parentNodeIndex = m_nodeParent[entity.index];

        if (parentNodeIndex != Constants::uint32_null_index)
        {
            return m_nodeToEntity[parentNodeIndex];
        }

        return Entity::null();
    }

    std::vector<Entity> SceneGraph::getChildren(Entity entity) const noexcept
    {
        std::vector<Entity> children;

        if (isPresent(entity))
        {
            auto find = m_childNodes.find(entity.index);

            if (find != m_childNodes.end())
            {
                for (auto childIndex : find->second)
                {
                    children.push_back(m_nodeToEntity[childIndex]);
                }
            }
        }

        return children;
    }

    uint32_t SceneGraph::getGpuBufferIndex(Entity entity) const noexcept
    {
        if (isPresent(entity))
        {
            return m_nodeGpuIndex[entity.index];
        }

        return Constants::uint32_null_index;
    }

    uint32_t SceneGraph::count() const noexcept
    {
        return m_activeCount;
    }

    uint32_t SceneGraph::storageSize() const noexcept
    {
        return static_cast<uint32_t>(m_nodeToEntity.size());
    }

    bool SceneGraph::isPresent(Entity entity) const noexcept
    {
        return
            !entity.isNull() &&
            (storageSize() > entity.index) &&
            (m_nodeToEntity[entity.index] == entity) &&
            (m_nodeOccupied[entity.index] == NodeState::Present);
    }

    void SceneGraph::ensureFit(uint32_t index)
    {
        // These two should be the same value, but just incase ...
        if (index == Constants::uint32_null_index)
        {
            return;
        }

        uint32_t count = index + 1;

        if (storageSize() >= count)
        {
            // Since all arrays grow together, we check just one to see if it can contain the specified index.
            return;
        }

        grow(count, m_nodeToEntity, Entity::null());
        grow(count, m_nodeParent, Constants::uint32_null_index);
        grow(count, m_nodeDepth, 0u);
        grow(count, m_nodeGpuIndex, Constants::uint32_null_index);
        grow(count, m_nodeOccupied, NodeState::Vacant);
    }

    void SceneGraph::updateEntity(uint32_t index, Entity entity, Entity parent, uint32_t depth, uint32_t gpuIndex)
    {
        m_nodeToEntity[index] = entity;
        m_nodeParent[index] = parent.index;
        m_nodeDepth[index] = depth;
        m_nodeGpuIndex[index] = gpuIndex;
        m_nodeOccupied[index] = (entity.isNull() ? NodeState::Vacant : NodeState::Present);
    }

    void SceneGraph::removeChildrenOf(Entity entity)
    {
        std::vector<uint32_t> descendents;
        std::stack<uint32_t> frontier;

        auto find = m_childNodes.find(entity.index);

        if (find == m_childNodes.end())
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

            find = m_childNodes.find(nodeIndex);

            if (find != m_childNodes.end())
            {
                for (auto grandchildIndex : find->second)
                {
                    frontier.push(grandchildIndex);
                }
            }
        }

        for (auto nodeIndex : descendents)
        {
            m_childNodes.erase(nodeIndex);
            updateEntity(nodeIndex, Entity::null(), Entity::null(), 0, Constants::uint32_null_index);
            m_activeCount--;
        }

        m_childNodes.erase(entity.index);
    }
}