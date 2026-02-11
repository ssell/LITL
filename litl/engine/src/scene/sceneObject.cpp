#include "litl-engine/scene/sceneObject.hpp"
#include <atomic>

namespace LITL::Engine
{
    namespace
    {
        static std::atomic<uint32_t> idStore{ 0 };
    }

    SceneObject::SceneObject()
        : m_id(++idStore)
    {

    }
}