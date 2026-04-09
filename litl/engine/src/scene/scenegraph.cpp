#include "litl-engine/scene/scenegraph.hpp"

namespace litl
{
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