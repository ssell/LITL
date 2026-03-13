#ifndef LITL_CORE_SERVICES_PROVIDER_H__
#define LITL_CORE_SERVICES_PROVIDER_H__

#include <any>
#include <memory>
#include <vector>

#include "litl-core/impl.hpp"
#include "litl-core/types.hpp"
#include "litl-core/services/serviceDescriptor.hpp"

namespace LITL::Core
{
    class ServiceProvider
    {
    public:

        explicit ServiceProvider(std::vector<ServiceDescriptor> descriptors);
        ~ServiceProvider();

        template<typename ServiceType>
        std::shared_ptr<ServiceType> get()
        {
            auto service = resolve(type_id<ServiceType>());

            if (!service.has_value())
            {
                // ruh roh
            }

            return std::any_cast<std::shared_ptr<ServiceType>>(service);
        }

    protected:

    private:

        std::any resolve(TypeId type);
        std::any resolveSingleton(ServiceDescriptor const& descriptor);
        ServiceFactoryResolver createResolver();

        ServiceDescriptor const* find(TypeId type) const noexcept;

        struct Impl;
        ImplPtr<Impl, 256> m_impl;
    };
}

#endif