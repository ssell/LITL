#ifndef LITL_ENGINE_SCENE_MANAGER_H__
#define LITL_ENGINE_SCENE_MANAGER_H__

#include <span>

#include "litl-core/impl.hpp"
#include "litl-core/authority.hpp"
#include "litl-engine/scene/sceneConfig.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;

    class World;
    class ServiceProvider;
    struct EntityChange;

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

        void setup(Authority<Engine> authority, ServiceProvider& services) noexcept;
        void createScene(SceneConfig const& config) noexcept;
        [[nodiscard]] uint32_t getSceneCount() const noexcept;
        [[nodiscard]] uint32_t getActiveSceneIndex() const noexcept;
        void setActiveScene(uint32_t index) noexcept;

        void processEntityChanges(Authority<EngineCallbacks> authority, World& world, std::span<EntityChange const> entityChanges) noexcept;
        void onPreRender(Authority<EngineCallbacks> authority, World& world) noexcept;

    protected:

    private:

        friend class Engine;

        struct Impl;
        ImplPtr<Impl, 152> m_impl;
    };
}

#endif