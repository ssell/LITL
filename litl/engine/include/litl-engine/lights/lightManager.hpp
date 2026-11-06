#ifndef LITL_ENGINE_LIGHT_MANAGER_H__
#define LITL_ENGINE_LIGHT_MANAGER_H__

#include "litl-core/authority.hpp"
#include "litl-core/math/types.hpp"
#include "litl-engine/lights/lightHandles.hpp"
#include "litl-engine/lights/directional/directionalLight.hpp"

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

        /// <summary>
        /// Sets the current ambient RGB color.
        /// </summary>
        /// <param name="rgb"></param>
        void setAmbientColor(color rgb) noexcept;

        /// <summary>
        /// Marks the directional light attached to the specified entity as active.
        /// </summary>
        void markActive(Authority<DirectionalLightSystem> auth, Entity entity, DirectionalLight const& light, vec3 toLightDir) noexcept;

        /// <summary>
        /// Retrieves the general light data buffer.
        /// This stores ambient light color and information for each light type (buffer, count, etc.).
        /// </summary>
        [[nodiscard]] GpuBuffer const* getLightDataBuffer() const noexcept;

        /// <summary>
        /// Returns the number of tracked directional lights.
        /// </summary>
        [[nodiscard]] uint32_t getDirectionalLightCount() const noexcept;

        /// <summary>
        /// Returns a copy of all directional light data and the entities they are attached to.
        /// </summary>
        [[nodiscard]] std::vector<DirectionalLightDataEntity> getDirectionalLightData() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif