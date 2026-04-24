#ifndef LITL_ENGINE_SCENE_H__
#define LITL_ENGINE_SCENE_H__

#include "litl-core/impl.hpp"
#include "litl-engine/scene/sceneAccessKey.hpp"
#include "litl-engine/scene/sceneConfig.hpp"

namespace litl
{
    class Scene
    {
    public:

        Scene(SceneConfig const& config);
        ~Scene();

        Scene(Scene const&) = delete;
        Scene& operator=(Scene const&) = delete;

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 768> m_impl;
    };
}

#endif