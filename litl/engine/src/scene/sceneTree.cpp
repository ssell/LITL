#include <queue>
#include <vector>

#include "litl-core/logging/logging.hpp"
#include "litl-engine/scene/sceneTree.hpp"
#include "litl-engine/scene/sceneObject.hpp"

namespace LITL::Engine
{
    struct SceneTree::Impl
    {
        std::queue<Core::RefPtr<SceneObject>> newObjects;
        std::vector<Core::RefPtr<SceneObject>> objects;
    };

    SceneTree::SceneTree()
        : m_pImpl(std::make_unique<SceneTree::Impl>())
    {
        m_pImpl->objects.reserve(512);
    }

    SceneTree::~SceneTree()
    {

    }

    void SceneTree::track(Core::RefPtr<SceneObject> pSceneObject)
    {
        m_pImpl->newObjects.push(pSceneObject);
    }

    void SceneTree::untrack(Core::RefPtr<SceneObject> pSceneObject)
    {
        // ... todo ...
        logError("!Unimplemented! SceneTree::untrack");
    }

    void SceneTree::onUpdate()
    {
        while (!m_pImpl->newObjects.empty())
        {
            auto newObj = m_pImpl->newObjects.front();
            m_pImpl->newObjects.pop();
            m_pImpl->objects.emplace_back(newObj);
            newObj->onSetup();
        }

        for (uint32_t i = 0; i < m_pImpl->objects.size(); ++i)
        {
            m_pImpl->objects[i]->onUpdate();
        }
    }

    void SceneTree::onRender(Renderer::CommandBuffer* pCommandBuffer)
    {
        for (uint32_t i = 0; i < m_pImpl->objects.size(); ++i)
        {
            m_pImpl->objects[i]->onRender(pCommandBuffer);
        }
    }
}