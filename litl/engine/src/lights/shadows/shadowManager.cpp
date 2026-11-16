#include <vector>

#include "litl-engine/lights/shadows/shadowManager.hpp"

namespace litl
{
    struct ShadowManager::Impl
    {
        std::vector<ShadowView> shadowViews;

        void setup(ServiceProvider& services) noexcept
        {
            // ... todo ...
        }

        void destroy() noexcept
        {
            // ... todo ...
        }

        void onPreRender() noexcept
        {
            // ... todo ...
        }
    };

    ShadowManager::ShadowManager()
        : m_pImpl(std::make_unique<Impl>())
    {

    }

    ShadowManager::~ShadowManager()
    {
        // ... required as this is stored in a unique_ptr in the service provider ...
    }

    void ShadowManager::setup(Authority<Engine> auth, ServiceProvider& services) noexcept
    {
        // ... todo ...
    }

    void ShadowManager::destroy(Authority<Engine> auth) noexcept
    {
        // ... todo ...
    }

    void ShadowManager::onPreRender(Authority<EngineCallbacks> auth) noexcept
    {
        // ... todo ...
    }

    std::span<ShadowView const> ShadowManager::getShadowViews() const noexcept
    {
        return m_pImpl->shadowViews;
    }
}