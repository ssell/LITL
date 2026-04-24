#include <variant>
#include <memory>
#include <vector>

#include "litl-core/assert.hpp"
#include "litl-engine/scene/sceneManager.hpp"

namespace litl
{
    struct SceneManager::Impl
    {
        std::vector<std::unique_ptr<Scene>> scenes;
        uint32_t activeIndex{ 0 };
    };

    SceneManager::SceneManager()
    {

    }

    SceneManager::~SceneManager()
    {

    }

    void SceneManager::createScene(SceneConfig const& config) noexcept
    {
        m_impl->scenes.push_back(std::make_unique<Scene>(config));
    }

    uint32_t SceneManager::getSceneCount() const noexcept
    {
        return static_cast<uint32_t>(m_impl->scenes.size());
    }

    uint32_t SceneManager::getActiveSceneIndex() const noexcept
    {
        return m_impl->activeIndex;
    }

    void SceneManager::setActiveScene(uint32_t index) noexcept
    {
        if (m_impl->activeIndex == index)
        {
            return;
        }

        LITL_ASSERT_MSG(index < getSceneCount(), "Invalid scene index specified for active scene.", );

        m_impl->activeIndex = index;

        // ... todo whatever is actually needed to swap to a different scene ...
    }
}