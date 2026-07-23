#include "litl-core/assert.hpp"
#include "litl-core/authority.hpp"
#include "litl-ecs/world.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/ecs/components/bounds.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-engine/objects/camera.hpp"
#include "litl-renderer/renderer.hpp"

namespace litl
{
    Scene::Scene(SceneConfig const& config, Renderer const* renderer, ObjectPool* objectPool)
    {
        switch (config.partition)
        {
        case ScenePartitionType::Null:
            m_partition.emplace<NullPartition>();
            break;

        case ScenePartitionType::UniformGrid:
            m_partition.emplace<UniformGridPartition>(config.uniformGridOptions);
            break;

        default:
            LITL_ASSERT_MSG(false, "Unsupported Scene Partition strategy.", );
        }

        m_pRenderer = renderer;
        m_transforms.reserve(1024u);
        m_cameras.setup(objectPool);
    }

    Scene::~Scene()
    {
        // ... needed as this is stored in a shared_ptr ...
    }

    void Scene::track(Entity entity, Transform const& transform) noexcept
    {
        track(entity, transform, bounds::AABB::fromCenterHalfExtents(transform.getPosition(), vec3{ 0.5f, 0.5f, 0.5f }));
    }

    void Scene::track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
    {
        m_graph.add(entity, transform);

        std::visit([&](auto& partition) 
        {
            partition.add(entity, bounds);
        }, m_partition);
    }

    void Scene::untrack(Entity entity) noexcept
    {
        m_graph.remove(entity);

        std::visit([&](auto& partition) 
        {
            partition.remove(entity);
        }, m_partition);
    }

    void Scene::update(Entity entity, bounds::AABB bounds) noexcept
    {
        std::visit([&](auto& partition) 
        { 
            partition.update(entity, bounds); 
        }, m_partition);
    }

    bool Scene::isPresent(Entity entity) const noexcept
    {
        return m_graph.isPresent(entity);
    }

    Entity Scene::getParent(Entity entity) const noexcept
    {
        return m_graph.getParent(entity);
    }

    void Scene::setParent(Entity child, Entity parent) noexcept
    {
        m_graph.setParent(child, parent);
    }

    std::vector<Entity> Scene::getChildren(Entity entity, bool recursive) const noexcept
    {
        return m_graph.getChildren(entity, recursive);
    }

    uint32_t Scene::getChildren(Entity entity, std::vector<Entity>& children, bool recursive) const noexcept
    {
        return m_graph.getChildren(entity, children, recursive);
    }

    uint32_t Scene::getGpuBufferIndex(Entity entity) const noexcept
    {
        return m_graph.getGpuBufferIndex(entity);
    }

    mat4 Scene::getWorldMatrix(Entity entity) const noexcept
    {
        uint32_t gpuBufferIndexForEntity = m_graph.getGpuBufferIndex(entity);

        if (gpuBufferIndexForEntity == Constants::uint32_null_index)
        {
            return {};
        }
        else
        {
            return m_transforms.getWorldMatrix(gpuBufferIndexForEntity);
        }
    }

    std::span<mat4 const> Scene::getWorldMatrices() const noexcept
    {
        return m_transforms.getWorldMatrices();
    }

    void Scene::onPreRender(Authority<SceneManager> authority, World& world) noexcept
    {
        m_graph.update();           // Update the graph to account for structural changes: create, destroy, reparent.

        // Update the world transforms for the frame.
        for (auto sortedIndex : m_graph.m_sortedNodes)
        {
            auto entity = m_graph.m_nodeToEntity[sortedIndex];
            auto localTransform = world.getComponent<Transform>(entity);

            if (localTransform.has_value())
            {
                // Calculate the new world matrix for the entity
                mat4 worldMatrix = localTransform->getWorldMatrix();
                uint32_t parentIndex = m_graph.m_nodeParent[sortedIndex];

                if (parentIndex != Constants::uint32_null_index)
                {
                    // This node has a parent. Use it to calculate the world transform.
                    uint32_t parentGpuIndex = m_graph.m_nodeGpuIndex[parentIndex];

                    if (parentGpuIndex != Constants::uint32_null_index)
                    {
                        worldMatrix = m_transforms.getWorldMatrix(parentGpuIndex) * worldMatrix;
                    }
                }

                // Set the world matrix in the scene transforms buffer
                uint32_t gpuIndex = m_graph.m_nodeGpuIndex[sortedIndex];

                if (gpuIndex != Constants::uint32_null_index)
                {
                    m_transforms.setWorldMatrix(gpuIndex, worldMatrix);
                }

                // Update the entity bounds in the scene partition
                auto localBounds = world.getComponent<LocalBounds>(entity);

                if (localBounds.has_value())
                {
                    // Update the scene partition with the world bounds
                    bounds::AABB calculatedWorldBounds = localBounds.value().bounds;
                    calculatedWorldBounds.min = worldMatrix * calculatedWorldBounds.min;
                    calculatedWorldBounds.max = worldMatrix * calculatedWorldBounds.max;

                    std::visit([&](auto& partition) 
                    { 
                        partition.update(entity, calculatedWorldBounds); 
                    }, m_partition);

                    // Update the WorldBounds component. This has no effect is the entity does not have the component already.
                    world.setComponent<WorldBounds>(entity, WorldBounds{
                        .bounds = calculatedWorldBounds,
                        .version = localTransform->getVersion()
                    });
                }
            }
        }

        // Update all cameras
        m_cameras.update();
        auto cameras = m_cameras.getCameras();

        for (auto* camera : cameras)
        {
            auto entity = camera->getEntity();
            auto gpuIndex = m_graph.getGpuBufferIndex(entity);

            if (gpuIndex != Constants::uint32_null_index)
            {
                if (camera->isMainCamera())
                {
                    // Set the aspect ratio to match the render target.
                    camera->setAspectRatio(m_pRenderer->getSwapchainDimensions().aspectRatio);
                }

                camera->update({}, m_transforms.getWorldMatrix(gpuIndex));
            }
        }
    }

    void Scene::query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
    {
        std::visit([&](auto& partition) 
        { 
            partition.query(aabb, entities); 
        }, m_partition);
    }

    void Scene::query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
    {
        std::visit([&](auto& partition) 
        {
            partition.query(sphere, entities); 
        }, m_partition);
    }

    void Scene::query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept
    {
        std::visit([&](auto& partition) 
        { 
            partition.query(frustum, entities); 
        }, m_partition);
    }

    void Scene::setMainCamera(CameraHandle handle) noexcept
    {
        m_cameras.setMainCamera(handle);
    }

    CameraHandle Scene::getMainCameraHandle() const noexcept
    {
        return m_cameras.getMainCameraHandle();
    }

    Camera* Scene::getMainCamera() const noexcept
    {
        return m_cameras.getMainCamera();
    }

    std::span<Camera*> Scene::getCameras() noexcept
    {
        return m_cameras.getCameras();
    }
}