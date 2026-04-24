#ifndef LITL_ENGINE_SCENE_MANAGER_H__
#define LITL_ENGINE_SCENE_MANAGER_H__

#include "litl-core/impl.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneConfig.hpp"

namespace litl
{
    /// <summary>
    /// Provides a public interface to the game scene(s).
    /// </summary>
    class SceneManager
    {
    public:

        SceneManager();
        ~SceneManager();

        SceneManager(SceneManager const&) = delete;
        SceneManager& operator=(SceneManager const&) = delete;

        void createScene(SceneConfig const& config) noexcept;
        [[nodiscard]] uint32_t getSceneCount() const noexcept;
        [[nodiscard]] uint32_t getActiveSceneIndex() const noexcept;
        void setActiveScene(uint32_t index) noexcept;

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 128> m_impl;
    };
}

#endif