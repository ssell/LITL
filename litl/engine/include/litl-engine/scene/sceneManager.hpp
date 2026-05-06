#ifndef LITL_ENGINE_SCENE_MANAGER_H__
#define LITL_ENGINE_SCENE_MANAGER_H__

#include <span>

#include "litl-core/impl.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/sceneConfig.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace litl
{
    class Engine;
    class World;
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

        void createScene(SceneConfig const& config) noexcept;
        [[nodiscard]] uint32_t getSceneCount() const noexcept;
        [[nodiscard]] uint32_t getActiveSceneIndex() const noexcept;
        void setActiveScene(uint32_t index) noexcept;

        void processEntityChanges(World& world, std::span<EntityChange const> entityChanges) noexcept;

    protected:

    private:

        friend class Engine;

        void setup(ServiceProvider& services) noexcept;

        struct Impl;
        ImplPtr<Impl, 88> m_impl;
    };
}

#endif