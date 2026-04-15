#ifndef LITL_ENGINE_SCENE_H__
#define LITL_ENGINE_SCENE_H__

#include "litl-core/impl.hpp"
#include "litl-engine/scene/sceneAccessKey.hpp"
#include "litl-engine/scene/partition/scenePartition.hpp"

namespace litl
{
    class Scene
    {
    public:

        Scene();
        ~Scene();

        Scene(Scene const&) = delete;
        Scene(Scene&&) = delete;
        Scene& operator=(Scene const&) = delete;

    protected:

    private:

        struct Impl;
        ImplPtr<Impl, 512> m_impl;
    };
}

#endif