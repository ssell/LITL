#ifndef LITL_ENGINE_LIGHTS_SHADOW_MANAGER_H__
#define LITL_ENGINE_LIGHTS_SHADOW_MANAGER_H__

#include <memory>
#include <span>

#include "litl-core/authority.hpp"
#include "litl-engine/lights/shadows/shadowView.hpp"

namespace litl
{
    class Engine;
    class EngineCallbacks;
    class ServiceProvider;
    class Camera;

    class ShadowManager
    {
    public:

        ShadowManager();
        ~ShadowManager();

        ShadowManager(ShadowManager const&) = delete;
        ShadowManager& operator=(ShadowManager const&) = delete;

        void setup(Authority<Engine> auth, ServiceProvider& services) noexcept;
        void destroy(Authority<Engine> auth) noexcept;
        void onPreRender(Authority<EngineCallbacks> auth) noexcept;

        [[nodiscard]] static bool cameraValidForShadows(Camera const* camera) noexcept;
        [[nodiscard]] std::span<ShadowView const> getShadowViews() const noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif