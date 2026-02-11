#ifndef LITL_ENGINE_H__
#define LITL_ENGINE_H__

#include "litl-core/impl.hpp"
#include "litl-renderer/rendererDescriptor.hpp"
#include "litl-engine/scene/sceneObject.hpp"

namespace LITL::Engine
{
    class Engine final
    {
    public:

        Engine(Renderer::RendererDescriptor const& rendererDescriptor);
        ~Engine();

        Engine(Engine&&) = delete;
        Engine(Engine const&) = delete;
        Engine& operator=(Engine&&) = delete;
        Engine& operator=(Engine const&) = delete;

        bool openWindow(char const* title, uint32_t width, uint32_t height) noexcept;
        bool shouldRun() noexcept;

        template<SceneObjectImpl T>
        void addSceneObject()
        {
            track(Core::RefPtr<SceneObject>(new T));
        }

        void run();

    protected:

    private:

        void track(Core::RefPtr<SceneObject> pSceneObject);

        void update();
        void render();

        struct Impl;
        Core::ImplPtr<Impl, 64> m_impl;
    };
}

#endif