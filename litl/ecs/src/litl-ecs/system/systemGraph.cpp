#include <unordered_set>
#include "litl-ecs/system/systemGraph.hpp"
#include "litl-ecs/system/system.hpp"

namespace LITL::ECS
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
        m_nodeGraph.addNode(m_systemNodes.size());
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

        return m_nodeGraph.addEdge(dependsOnIndex.value(), dependentIndex.value());
    }

    bool SystemGraph::setPlacementHint(SystemTypeId systemTypeId, SystemNodePlacementHint placement) noexcept
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
        // First, add the edges between implicit dependencies. 
        // An implicit dependency is when a system has a conflict with another.
        // This is due to one system writing to a component that then another
        // system either needs to read or write to as well.
        addImplicitDependencies();

        // If the DAG sort returns false, then it indicates a cycle was detected.
        if (!m_nodeGraph.sort())
        {
            // Let the caller (system manager) handle this particular error.
            return false;
        }

        // ... todo actually use the DAG ...

        return true;
    }

    void SystemGraph::addImplicitDependencies() noexcept
    {
        // Track the readers and most recent writer for each component type.
        std::unordered_map<ComponentTypeId, ComponentAccessors> componentAccessMap;

        // For each system, iterate the components it access (both read and write)
        // and add dependencies (in the form of DAG edges) to other systems based on their usage.
        for (uint32_t i = 0; i < static_cast<uint32_t>(m_systemNodes.size()); ++i)
        {
            auto& systemNode = m_systemNodes[i];
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

                    accessors.readers.push_back(i);
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
                    accessors.lastWriter = i;
                }
            }

            for (auto dependency : dependencies)
            {
                m_nodeGraph.addEdge(dependency, i);
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

    bool SystemGraph::run(World& world, float dt, std::vector<System*> const& systems)
    {
        for (auto& sortedNode : m_nodeGraph.getSorted())
        {
            systems[m_systemNodes[sortedNode].systemId]->run(world, dt);
        }

        return false;
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

    Math::DirectedAcyclicGraph const& SystemGraph::getNodeGraph() const noexcept
    {
        return m_nodeGraph;
    }
}