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
    class GpuBuffer;
    class DirectionalLightSystem;
    struct DirectionalLight;

    class LightManager
    {
    public:

        LightManager();
        ~LightManager();

        LightManager(LightManager const&) = delete;
        LightManager& operator=(LightManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void onPreRender(Authority<EngineCallbacks> auth, uint32_t framesInFlight, uint32_t framesInFlightIndex) noexcept;

        [[nodiscard]] GpuBuffer const* getLightDataBuffer() const noexcept;

        void setAmbientColor(color rgb) noexcept;

        /// <summary>
        /// Marks the directional light attached to the specified entity as active.
        /// </summary>
        /// <param name="auth"></param>
        /// <param name="entity"></param>
        /// <param name="light"></param>
        /// <param name="toLightDir"></param>
        void markActive(Authority<DirectionalLightSystem> auth, Entity entity, DirectionalLight const& light, vec3 toLightDir) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif