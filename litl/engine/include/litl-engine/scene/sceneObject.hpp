#ifndef LITL_ENGINE_SCENE_OBJECT_H__
#define LITL_ENGINE_SCENE_OBJECT_H__

#include <cstdint>

#include "litl-core/refPtr.hpp"
#include "litl-renderer/commands/commandBuffer.hpp"

namespace LITL::Engine
{
    class SceneObject : public Core::RefCounted
    {
    public:

        SceneObject();
        ~SceneObject() = default;

        virtual void onSetup() = 0;
        virtual void onUpdate() = 0;
        virtual void onRender(Renderer::CommandBuffer* pCommandBuffer) = 0;

        constexpr uint32_t getId()
        {
            return m_id;
        }

    protected:

    private:

        const uint32_t m_id;
    };

    template<typename T>
    concept SceneObjectImpl = std::derived_from<T, SceneObject>;
}

#endif