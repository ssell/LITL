#ifndef LITL_ENGINE_LIGHT_MANAGER_H__
#define LITL_ENGINE_LIGHT_MANAGER_H__

#include "litl-core/authority.hpp"
#include "litl-core/math/types.hpp"
#include "litl-engine/lights/lightHandles.hpp"

#include <memory>

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class ServiceProvider;

    struct DirectionalLightDescriptor
    {
        color color{ colors::White };
        float intensity{ 1.0f };
    };

    class LightManager
    {
    public:

        LightManager();
        ~LightManager();

        LightManager(LightManager const&) = delete;
        LightManager& operator=(LightManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void onPreRender(Authority<EngineCallbacks> auth) noexcept;

        DirectionalLightHandle createDirectionalLight(DirectionalLightDescriptor const& descriptor, vec3 position, quat rotation) noexcept;
        void destroyDirectionalLight(DirectionalLightHandle handle) noexcept;
        void deferDestroyDirectionalLight(DirectionalLightHandle handle) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif