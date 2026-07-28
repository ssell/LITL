#include "litl-core/assert.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/scene/scene.hpp"

namespace litl
{
    SceneView::SceneView()
        : m_pActiveScene(nullptr)
    {

    }

    SceneView::~SceneView()
    {

    }

    void SceneView::setViewedScene(std::shared_ptr<Scene> scene) noexcept
    {
        m_pActiveScene = scene;
    }

    bool SceneView::isPresent(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::isPresent on a null scene.", false);
        return m_pActiveScene->isPresent(entity);
    }

    void SceneView::track(Entity entity, Transform const& transform) noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::track on a null scene.", );

        if (!m_pActiveScene->isPresent(entity))
        {
            m_pActiveScene->track(entity, transform);
        }
    }

    void SceneView::track(Entity entity, Transform const& transform, bounds::AABB bounds) noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::track on a null scene.", );

        if (!m_pActiveScene->isPresent(entity))
        {
            m_pActiveScene->track(entity, transform, bounds);
        }
    }

    Entity SceneView::getParent(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getParent on a null scene.", Entity::null());
        return m_pActiveScene->getParent(entity);
    }

    std::vector<Entity> SceneView::getChildren(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getChildren on a null scene.", {});
        return m_pActiveScene->getChildren(entity);
    }

    uint32_t SceneView::getGpuBufferIndex(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getGpuBufferIndex on a null scene.", Constants::uint32_null_index);
        return m_pActiveScene->getGpuBufferIndex(entity);
    }

    mat4 SceneView::getWorldMatrix(Entity entity) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getGpuBufferIndex on a null scene.", {});
        return m_pActiveScene->getWorldMatrix(entity);
    }

    std::span<mat4 const> SceneView::getWorldMatrices() const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getEntityTransformsBufferAddress on a null scene.", {});
        return m_pActiveScene->getWorldMatrices();
    }

    vec3 SceneView::getWorldPosition(Entity entity) const noexcept
    {
        return m_pActiveScene->getWorldMatrix(entity).position();
    }

    void SceneView::query(bounds::AABB aabb, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(aabb,) on a null scene.", );
        m_pActiveScene->query(aabb, entities, sorted);
    }

    void SceneView::query(bounds::AABB aabb, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(aabb,componentType) on a null scene.", );
        m_pActiveScene->query(aabb, componentType, entities, sorted);
    }

    void SceneView::query(bounds::Sphere sphere, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(sphere,) on a null scene.", );
        m_pActiveScene->query(sphere, entities, sorted);
    }

    void SceneView::query(bounds::Sphere sphere, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(sphere,componentType) on a null scene.", );
        m_pActiveScene->query(sphere, componentType, entities, sorted);
    }

    void SceneView::query(bounds::Frustum frustum, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(frustum,) on a null scene.", );
        m_pActiveScene->query(frustum, entities, sorted);
    }

    void SceneView::query(bounds::Frustum frustum, ComponentTypeId componentType, std::vector<PartitionQueryResult>& entities, bool sorted) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(frustum,componentType) on a null scene.", );
        m_pActiveScene->query(frustum, componentType, entities, sorted);
    }

    void SceneView::setMainCamera(CameraHandle handle) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::setMainCamera on a null scene.", );
        m_pActiveScene->setMainCamera(handle);
    }

    CameraHandle SceneView::getMainCameraHandle() const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getMainCameraHandle on a null scene.", {});
        return m_pActiveScene->getMainCameraHandle();
    }

    Camera* SceneView::getMainCamera() const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getMainCamera on a null scene.", nullptr);
        return m_pActiveScene->getMainCamera();
    }

    std::span<Camera*> SceneView::getCameras() noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::getCameras on a null scene.", {});
        return m_pActiveScene->getCameras();
    }
}