#ifndef LITL_ENGINE_SCENE_GRAPH_H__
#define LITL_ENGINE_SCENE_GRAPH_H__

#include <memory>

#include "litl-core/math.hpp"
#include "litl-ecs/entity/entity.hpp"

namespace litl
{
    /// <summary>
    /// The scene graph has three main responsibilities:
    /// 
    ///   1. Providing an iterable structure for entity hierarchy (parent -> child)
    ///   2. Providing random access to determine entity direct relatives (entity -> parent, entity -> children)
    ///   3. Maintaining the entity index into the World Matrix buffer.
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