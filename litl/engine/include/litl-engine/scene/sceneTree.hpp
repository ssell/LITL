#ifndef LITL_ENGINE_SCENE_TREE_H__
#define LITL_ENGINE_SCENE_TREE_H__

#include <memory>

#include "litl-core/refPtr.hpp"
#include "litl-engine/scene/sceneObject.hpp"

namespace LITL::Engine
{
    /// <summary>
    /// A scene-tree in name only. But need something to get the engine going.
    /// </summary>
    class SceneTree
    {
    public:

        SceneTree();
        ~SceneTree();

        SceneTree(SceneTree const&) = delete;
        SceneTree& operator=(SceneTree const&) = delete;

        void track(Core::RefPtr<SceneObject> pSceneObject);
        void untrack(Core::RefPtr<SceneObject> pSceneObject);

        void onUpdate();
        void onRender(Renderer::CommandBuffer* pCommandBuffer);

    protected:

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif