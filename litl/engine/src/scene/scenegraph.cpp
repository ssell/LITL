#include "litl-engine/scene/scenegraph.hpp"

namespace litl
{
    /// <summary>
    /// Entity iteration and ordering.
    /// </summary>
    struct SceneGraphTree
    {

    };

    /// <summary>
    /// Entity random access.
    /// </summary>
    struct SceneGraphArray
    {

    };

    struct SceneGraph::Impl
    {

    };

    SceneGraph::SceneGraph()
        : m_pImpl(std::make_unique<SceneGraph::Impl>())
    {

    }

    SceneGraph::~SceneGraph()
    {

    }
}