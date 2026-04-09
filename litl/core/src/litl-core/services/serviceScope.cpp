#include <unordered_map>

#include "litl-core/services/serviceScope.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace litl
{
    struct ServiceScope::Impl
    {
        ServiceProvider* provider;
        std::unordered_map<TypeId, std::any> scopedServices;
    };

    ServiceScope::ServiceScope(ServiceProvider* provider)
    {
        m_impl->provider = provider;
        m_impl->scopedServices.reserve(provider->size());
    }

    ServiceScope::~ServiceScope()
    {

    }

    std::any ServiceScope::resolve(TypeId type) noexcept
    {
        auto descriptor = m_impl->provider->find(type);

        if (descriptor == nullptr)
        {
            return {};
        }

        if (descriptor->lifetime == ServiceLifetime::Scoped)
        {
            auto findScoped = m_impl->scopedServices.find(descriptor->type);

            if (findScoped != m_impl->scopedServices.end())
            {
                return findScoped->second;
            }
            else
            {
                auto scopedInstance = descriptor->factory(m_impl->provider->createResolver());
                m_impl->scopedServices[descriptor->type] = scopedInstance;
                return scopedInstance;
            }
        }
        else
        {
            return m_impl->provider->resolve(type);
        }
    }
}