#ifndef LITL_ENGINE_SCENE_MANAGER_H__
#define LITL_ENGINE_SCENE_MANAGER_H__

#include "litl-core/impl.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneConfig.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace litl
{
    class Engine;

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

        friend class Engine;

        void setup(ServiceProvider& services) noexcept;

        struct Impl;
        ImplPtr<Impl, 64> m_impl;
    };
}

#endif