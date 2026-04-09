#ifndef LITL_ENGINE_SCENE_GRAPH_H__
#define LITL_ENGINE_SCENE_GRAPH_H__

#include <memory>

#include "litl-core/math.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    /// <summary>
    /// Responsible for maintaining an iterable mapping of entity parent/child relationships.
    /// </summary>
    class SceneGraph
    {
    public:

        SceneGraph();
        ~SceneGraph();

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif