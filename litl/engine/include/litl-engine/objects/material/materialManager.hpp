#ifndef LITL_ENGINE_OBJECTS_MATERIAL_MANAGER_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_MANAGER_H__

#include <cstdint>
#include <vector>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class ServiceProvider;
    class ObjectPool;

    /// <summary>
    /// Materials are unique among Engine objects in that they require triggered updates throughout a frame.
    /// This class exists to ensure all Materials receive their required triggers, but it does not own them.
    /// </summary>
    class MaterialManager
    {
    public:

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void onFrameStart(Authority<EngineCallbacks> auth, uint32_t frame, uint32_t frameIndex) noexcept;
        void onPreRender(Authority<EngineCallbacks> auth) const noexcept;

    private:

        ObjectPool* m_pObjectPool{ nullptr };
        std::vector<MaterialHandle> m_materialHandles;
    };
}

#endif