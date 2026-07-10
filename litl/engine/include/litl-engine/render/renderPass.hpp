#ifndef LITL_ENGINE_RENDER_PASS_H__
#define LITL_ENGINE_RENDER_PASS_H__

#include <vector>
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    class Camera;
    class Renderer;

    class RenderPass
    {
    public:

        void render(Renderer* renderer, Camera* camera, std::vector<Entity> const& entities) const noexcept;
    };
}

#endif