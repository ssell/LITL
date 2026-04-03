#ifndef LITL_ENGINE_ECS_RENDER_SYSTEM_H__
#define LITL_ENGINE_ECS_RENDER_SYSTEM_H__

#include "litl-engine/ecs/common.hpp"
#include "litl-renderer/renderer.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// 
    /// </summary>
    struct RenderSystem
    {
        void setup(Core::ServiceProvider& services)
        {
            m_pRenderer = services.get<Renderer::Renderer>();
            assert(m_pRenderer != nullptr);
        }

        void update(ECS::EntityCommands& commands, float dt, Position& position)
        {

        }

    private:

        std::shared_ptr<Renderer::Renderer> m_pRenderer{ nullptr };
    };
}

#endif