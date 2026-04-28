#include "litl-core/assert.hpp"
#include "litl-engine/scene/sceneView.hpp"
#include "litl-engine/scene/sceneManager.hpp"

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

    void SceneView::query(bounds::AABB aabb, std::vector<Entity>& entities) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(aabb,) on a null scene.", );
        m_pActiveScene->query(aabb, entities);
    }

    void SceneView::query(bounds::Sphere sphere, std::vector<Entity>& entities) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(sphere,) on a null scene.", );
        m_pActiveScene->query(sphere, entities);
    }

    void SceneView::query(bounds::Frustum frustum, std::vector<Entity>& entities) const noexcept
    {
        LITL_ASSERT_MSG((m_pActiveScene != nullptr), "Attempting to use SceneView::query(frustum,) on a null scene.", );
        m_pActiveScene->query(frustum, entities);
    }
}