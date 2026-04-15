#include "litl-core/assert.hpp"
#include "litl-engine/scene/scene.hpp"
#include "litl-engine/scene/scenegraph.hpp"

namespace litl
{
    struct Scene::Impl
    {
        SceneGraph graph;
        // ... todo scene partition ...
    };

    Scene::Scene()
    {

    }

    Scene::~Scene()
    {

    }
}