#include <unordered_set>

#include "litl-core/job/jobFence.hpp"
#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"

namespace litl
{
    /// <summary>
    /// For each component referenced by a system in the graph,
    /// this tracks the latest writer to the component as well as all current readers.
    /// 
    /// This is used to help construct a final minimal DAG.
    /// </summary>
    struct ComponentAccessors
    {
        /// <summary>
        /// The index of the system (not the SystemTypeId) that is the most "recent" writer to the component.
        /// </summary>
        std::optional<uint32_t> lastWriter;

        /// <summary>
        /// The indices of the systems that read the component.
        /// </summary>
        std::vector<uint32_t> readers;
    };

    SystemGraph::SystemGraph()
    {

    }

    SystemGraph::~SystemGraph()
    {

    }

    void SystemGraph::add(SystemTypeId systemTypeId, std::vector<SystemComponentInfo> const& componentInfo) noexcept
    {
        m_systemNodes.emplace_back(systemTypeId, componentInfo);
    }

    bool SystemGraph::addDependency(SystemTypeId dependentSystem, SystemTypeId dependsOnSystem) noexcept
    {
        auto dependentIndex = findSystemIndex(dependentSystem);
        auto dependsOnIndex = findSystemIndex(dependsOnSystem);

        if (!dependentIndex.has_value() || !dependsOnIndex.has_value())
        {
            return false;
        }

        m_systemNodes[dependsOnIndex.value()].outgoing.insert(dependentIndex.value());

        return true;
    }

    bool SystemGraph::setPlacementHint(SystemTypeId systemTypeId, SystemPlacementHint placement) noexcept
    {
        auto systemIndex = findSystemIndex(systemTypeId);

        if (!systemIndex.has_value())
        {
            return false;
        }

        m_systemNodes[systemIndex.value()].placement = placement;

        return true;
    }

    bool SystemGraph::build() noexcept
    {
        m_nodeGraph = {};

        std::vector<uint32_t> sortedNodes(m_systemNodes.size());

        for (uint32_t i = 0; i < static_cast<uint32_t>(m_systemNodes.size()); ++i)
        {
            sortedNodes[i] = i;
        }

        std::stable_sort(sortedNodes.begin(), sortedNodes.end(), [this](uint32_t a, uint32_t b) -> bool
            {
                return m_systemNodes[a] < m_systemNodes[b];
            });

        for (auto sortedNodeIndex : sortedNodes)
        {
            m_nodeGraph.addNode(sortedNodeIndex);
        }

        addExplicitDependencies();
        addImplicitDependencies();
        applyPlacementHints();

        // If the DAG sort returns false, then it indicates a cycle was detected.
        return m_nodeGraph.sort();
    }

    void SystemGraph::applyPlacementHints() noexcept
    {
        std::vector<uint32_t> firstNodes;
        std::vector<uint32_t> lastNodes;
        std::vector<uint32_t> rootNodes;
        std::vector<uint32_t> leafNodes;

        for (uint32_t i = 0; i < static_cast<uint32_t>(m_systemNodes.size()); ++i)
        {
            if (m_systemNodes[i].placement == SystemPlacementHint::First)
            {
                firstNodes.push_back(i);
            }
            else if (m_systemNodes[i].placement == SystemPlacementHint::Last)
            {
                lastNodes.push_back(i);
            }
            else
            {
                if (!m_nodeGraph.hasIncoming(i))
                {
                    rootNodes.push_back(i);
                }

                if (!m_nodeGraph.hasOutgoing(i))
                {
                    leafNodes.push_back(i);
                }
            }
        }

        // Set all "first" nodes as direct dependencies of all 0-ingress/root nodes
        for (auto& first : firstNodes)
        {
            for (auto& root : rootNodes)
            {
                m_nodeGraph.addEdge(first, root);
            }
        }

        // Set all "last" nodes as direct dependents on all 0-egress/leaf nodes
        for (auto& last : lastNodes)
        {
            for (auto& leaf : leafNodes)
            {
                m_nodeGraph.addEdge(leaf, last);
            }
        }
    }

    void SystemGraph::addExplicitDependencies() noexcept
    {
        for (auto i = 0; i < m_systemNodes.size(); ++i)
        {
            for (auto& outgoing : m_systemNodes[i].outgoing)
            {
                m_nodeGraph.addEdge(i, outgoing);
            }
        }
    }

    void SystemGraph::addImplicitDependencies() noexcept
    {
        // Track the readers and most recent writer for each component type.
        std::unordered_map<ComponentTypeId, ComponentAccessors> componentAccessMap;

        auto& nodeIndices = m_nodeGraph.getUnsorted();

        // For each system, iterate the components it access (both read and write)
        // and add dependencies (in the form of DAG edges) to other systems based on their usage.
        for (uint32_t i = 0; i < static_cast<uint32_t>(nodeIndices.size()); ++i)
        {
            auto nodeIndex = nodeIndices[i];
            auto& systemNode = m_systemNodes[nodeIndex];
            std::unordered_set<uint32_t> dependencies;
            // ^ set to provide deduplication. may have redundant/duplicate dependencies resulting from systems writing to multiple shared component types.

            for (auto& component : systemNode.componentInfo)
            {
                auto& accessors = componentAccessMap[component.id];

                if (component.readonly)
                {
                    // If we read a component, we must run after the most recent writer.
                    if (accessors.lastWriter.has_value())
                    {
                        dependencies.insert(accessors.lastWriter.value());
                    }

                    accessors.readers.push_back(nodeIndex);
                }
                else
                {
                    // If we write to a component, we must run after all current read-only systems and the most recent write system.
                    for (auto readerIndex : accessors.readers)
                    {
                        dependencies.insert(readerIndex);
                    }

                    if (accessors.lastWriter.has_value())
                    {
                        dependencies.insert(accessors.lastWriter.value());
                    }

                    // Can clear "current" readers as any future writers will implicitly depend on them by having to depend on the current system.
                    // This is the part (along with the dedupe) that produces a minimized DAG.
                    accessors.readers.clear();
                    accessors.lastWriter = nodeIndex;
                }
            }

            for (auto dependency : dependencies)
            {
                m_nodeGraph.addEdge(dependency, nodeIndex);
            }
        }
    }

    bool SystemGraph::doesComponentAccessConflict(SystemNode& a, SystemNode& b) const noexcept
    {
        for (auto& aComponentInfo : a.componentInfo)
        {
            for (auto& bComponentInfo : b.componentInfo)
            {
                if ((aComponentInfo.id == bComponentInfo.id) &&
                    (!aComponentInfo.readonly || !bComponentInfo.readonly))
                {
                    // Same component needed AND one or both systems require write-access.
                    return true;
                }
            }
        }

        return false;
    }

    void SystemGraph::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto& sortedNode : m_nodeGraph.getSorted())
        {
            systems[m_systemNodes[sortedNode].systemId]->run(world, dt);
        }
    }

    std::optional<uint32_t> SystemGraph::findSystemIndex(SystemTypeId systemTypeId) const noexcept
    {
        for (auto i = 0; i < m_systemNodes.size(); ++i)
        {
            if (m_systemNodes[i].systemId == systemTypeId)
            {
                return i;
            }
        }

        return std::nullopt;
    }

    DirectedAcyclicGraph const& SystemGraph::getNodeGraph() const noexcept
    {
        return m_nodeGraph;
    }

    SystemNode const& SystemGraph::getNode(uint32_t index) const noexcept
    {
        assert(index <= m_systemNodes.size());
        return m_systemNodes[index];
    }
}