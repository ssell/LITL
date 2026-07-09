#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <memory>
#include <vector>

#include "litl-ecs/entity/entity.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace litl
{
    class Camera;
    class Renderer;

    class RenderPass
    {
    public:

        void setup(ServiceProvider& services) noexcept;
        void onRender() noexcept;

    private:

        void render(Camera* camera, std::vector<Entity> const& entities) noexcept;

        std::shared_ptr<Renderer> m_pRenderer = nullptr;
    };
}

#endif