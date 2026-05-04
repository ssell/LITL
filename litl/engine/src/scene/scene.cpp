#include <variant>

#include "litl-core/assert.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneGraph.hpp"
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
        SceneGraph graph;
        ScenePartitionVariant partition;

        void add(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
        {
            graph.add(entity, transform);
            std::visit([&](auto& partition) { partition.add(entity, bounds); }, partition);
        }

        void remove(Entity entity) noexcept
        {
            graph.remove(entity);
            std::visit([&](auto& partition) { partition.remove(entity); }, partition);
        }

        void update(Entity entity, bounds::AABB bounds) noexcept
        {
            std::visit([&](auto& partition) { partition.update(entity, bounds); }, partition);
        }

        [[nodiscard]] bool isPresent(Entity entity) const noexcept
        {
            return graph.isPresent(entity);
        }

        [[nodiscard]] Entity getParent(Entity entity) const noexcept
        {
            return graph.getParent(entity);
        }

        void setParent(Entity child, Entity parent) noexcept
        {
            graph.setParent(child, parent);
        }

        [[nodiscard]] std::vector<Entity> getChildren(Entity entity) const noexcept
        {
            return graph.getChildren(entity);
        }

        [[nodiscard]] uint32_t getGpuBufferIndex(Entity entity) const noexcept
        {
            return graph.getGpuBufferIndex(entity);
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

        void sync() noexcept
        {
            graph.update();
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
    }

    Scene::~Scene()
    {

    }

    void Scene::add(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
    {
        m_impl->add(entity, transform, bounds);
    }

    void Scene::remove(Entity entity) noexcept
    {
        m_impl->remove(entity);
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

    std::vector<Entity> Scene::getChildren(Entity entity) const noexcept
    {
        return m_impl->getChildren(entity);
    }

    uint32_t Scene::getGpuBufferIndex(Entity entity) const noexcept
    {
        return m_impl->getGpuBufferIndex(entity);
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

    void Scene::sync() noexcept
    {
        m_impl->sync();
    }
}