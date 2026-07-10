#ifndef LITL_ENGINE_CALLBACKS_H__
#define LITL_ENGINE_CALLBACKS_H__

#include <memory>
#include <span>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-ecs/frameCallbacks.hpp"
#include "litl-ecs/system/systemGroup.hpp"
#include "litl-engine/scene/sceneManager.hpp"

namespace litl
{
    class Engine;
    class RenderManager;

    class EngineCallbacks
    {
    public:

        void setup(Authority<Engine> authority, ServiceProvider& services, std::shared_ptr<FrameCallbacks> userCallbacks) noexcept;
        std::shared_ptr<FrameCallbacks> getFrameCallbacks() noexcept;

    protected:

    private:

        std::shared_ptr<World> m_pWorld{ nullptr };
        std::shared_ptr<SceneManager> m_pSceneManager{ nullptr };
        std::shared_ptr<RenderManager> m_pRenderManager{ nullptr };
        std::shared_ptr<FrameCallbacks> m_pFrameCallbacks{ std::make_shared<FrameCallbacks>() };
        std::shared_ptr<FrameCallbacks> m_pUserFrameCallbacks{ nullptr };
    };
}

#endif