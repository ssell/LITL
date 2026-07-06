#include <variant>

#include "litl-core/assert.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneGraph.hpp"
#include "litl-engine/scene/sceneTransforms.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"
#include "litl-engine/scene/partition/nullPartition.hpp"
#include "litl-engine/scene/partition/uniformGridPartition.hpp"

namespace litl
{
    using ScenePartitionVariant = std::variant<
        NullPartition,
        UniformGridPartition
        /* add future partition strategies here */
    >;

    struct Scene::Impl
    {
        SceneTransforms transforms;
        SceneGraph graph;
        ScenePartitionVariant partition;

        void track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
        {
            graph.add(entity, transform);
            std::visit([&](auto& partition) { partition.add(entity, bounds); }, partition);
        }

        void untrack(Entity entity) noexcept
        {
            graph.remove(entity);
            std::visit([&](auto& partition) { partition.remove(entity); }, partition);
        }

        void update(Entity entity, bounds::AABB bounds) noexcept
        {
            std::visit([&](auto& partition) { partition.update(entity, bounds); }, partition);
        }

        bool isPresent(Entity entity) const noexcept
        {
            return graph.isPresent(entity);
        }

        Entity getParent(Entity entity) const noexcept
        {
            return graph.getParent(entity);
        }

        void setParent(Entity child, Entity parent) noexcept
        {
            graph.setParent(child, parent);
        }

        std::vector<Entity> getChildren(Entity entity, bool recursive) const noexcept
        {
            return graph.getChildren(entity, recursive);
        }

        uint32_t getChildren(Entity entity, std::vector<Entity>& children, bool recursive) const noexcept
        {
            return graph.getChildren(entity, children, recursive);
        }

        uint32_t getGpuBufferIndex(Entity entity) const noexcept
        {
            return graph.getGpuBufferIndex(entity);
        }

        mat4 getWorldMatrix(Entity entity) const noexcept
        {
            uint32_t gpuBufferIndexForEntity = graph.getGpuBufferIndex(entity);

            if (gpuBufferIndexForEntity == Constants::uint32_null_index)
            {
                return {};
            }
            else
            {
                return transforms.getWorldMatrix(gpuBufferIndexForEntity);
            }
        }

        void query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
        {
            std::visit([&](auto& partition) { partition.query(aabb, entities); }, partition);
        }

        void query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
        {
            std::visit([&](auto& partition) { partition.query(sphere, entities); }, partition);
        }

        void query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept
        {
            std::visit([&](auto& partition) { partition.query(frustum, entities); }, partition);
        }

        void onPreRender(World const& world) noexcept
        {
            // Update the graph to account for structural changes: create, destroy, reparent.
            graph.update();

            // Update the world transforms for the frame.
            for (auto sortedIndex : graph.m_sortedNodes)
            {
                auto localTransform = world.getComponent<Transform>(graph.m_nodeToEntity[sortedIndex]);

                if (localTransform.has_value())
                {
                    mat4 worldMatrix = localTransform->getWorldMatrix();
                    uint32_t parentIndex = graph.m_nodeParent[sortedIndex];

                    if (graph.m_nodeParent[sortedIndex] != Constants::uint32_null_index)
                    {
                        // This node has a parent. Use it to calculate the world transform.
                        worldMatrix = transforms.getWorldMatrix(graph.m_nodeGpuIndex[parentIndex]) * worldMatrix;
                    }

                    transforms.setWorldMatrix(graph.m_nodeGpuIndex[sortedIndex], worldMatrix);
                }

            }

            // Update the scene partition based on the newly minted transforms.
            // ... todo ...
        }
    };

    Scene::Scene(SceneConfig const& config)
    {
        switch (config.partition)
        {
        case ScenePartitionType::UniformGrid:
            m_impl->partition.emplace<UniformGridPartition>(config.uniformGridOptions);
            break;

        default:
            LITL_ASSERT_MSG(false, "Unsupported Scene Partition strategy.", );
        }

        m_impl->transforms.reserve(1024u);
    }

    Scene::~Scene()
    {

    }

    void Scene::track(Entity entity, Transform const& transform) noexcept
    {
        m_impl->track(entity, transform, bounds::AABB::fromCenterHalfExtents(transform.getPosition(), vec3{0.5f, 0.5f, 0.5f}));
    }

    void Scene::track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
    {
        m_impl->track(entity, transform, bounds);
    }

    void Scene::untrack(Entity entity) noexcept
    {
        m_impl->untrack(entity);
    }

    void Scene::update(Entity entity, bounds::AABB bounds) noexcept
    {
        m_impl->update(entity, bounds);
    }

    bool Scene::isPresent(Entity entity) const noexcept
    {
        return m_impl->isPresent(entity);
    }

    Entity Scene::getParent(Entity entity) const noexcept
    {
        return m_impl->getParent(entity);
    }

    void Scene::setParent(Entity child, Entity parent) noexcept
    {
        m_impl->setParent(child, parent);
    }

    std::vector<Entity> Scene::getChildren(Entity entity, bool recursive) const noexcept
    {
        return m_impl->getChildren(entity, recursive);
    }

    uint32_t Scene::getChildren(Entity entity, std::vector<Entity>& children, bool recursive) const noexcept
    {
        return m_impl->getChildren(entity, children, recursive);
    }

    uint32_t Scene::getGpuBufferIndex(Entity entity) const noexcept
    {
        return m_impl->getGpuBufferIndex(entity);
    }

    void Scene::onPreRender(Authority<SceneManager> authority, World const& world) noexcept
    {
        m_impl->onPreRender(world);
    }

    void Scene::query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(aabb, entities);
    }

    void Scene::query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(sphere, entities);
    }

    void Scene::query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept
    {
        m_impl->query(frustum, entities);
    }

    mat4 Scene::getWorldMatrix(Entity entity) const noexcept
    {
        return m_impl->getWorldMatrix(entity);
    }
}