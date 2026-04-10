#include "litl-engine/scene/scenegraph.hpp"
#include "litl-core/containers/pagedVector.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    struct SceneGraph::Impl
    {
        Impl(size_t initialCapacity)
        {
            assert(initialCapacity > 0);

            entityToNode.reserve(initialCapacity);
            nodeToEntity.reserve(initialCapacity);
            nodeParent.reserve(initialCapacity);
            nodeDepth.reserve(initialCapacity);
            nodeGpuIndex.reserve(initialCapacity);
            treeEdgeParent.reserve(initialCapacity);
            treeEdgeChild.reserve(initialCapacity);
            sortedNodes.reserve(initialCapacity);
        }

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
        // Random access lookup
        // ---------------------------------------------------------------------------------

        /// <summary>
        /// EntityId -> Node Index
        /// </summary>
        std::vector<uint32_t> entityToNode;

        /// <summary>
        /// Node Index -> EntityId
        /// </summary>
        std::vector<Entity> nodeToEntity;

        // ---------------------------------------------------------------------------------
        // Scene Node in parallel arrays
        // ---------------------------------------------------------------------------------

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
    };

    SceneGraph::SceneGraph()
        : m_pImpl(std::make_unique<SceneGraph::Impl>(1024))
    {

    }

    SceneGraph::~SceneGraph()
    {

    }

    void SceneGraph::track(Entity entity, SceneGraphAccessKey)
    {
        // ... todo ...
    }

    void SceneGraph::setParent(Entity child, Entity parent, SceneGraphAccessKey)
    {
        // ... todo ...
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
        assert(entity.index < m_pImpl->entityToNode.size());

        uint32_t nodeIndex = m_pImpl->entityToNode[entity.index];
        uint32_t parentNodeIndex = m_pImpl->nodeParent[nodeIndex];

        return m_pImpl->nodeToEntity[parentNodeIndex];
    }
}