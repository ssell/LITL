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
    class ServiceScope;

    /// <summary>
    /// Instantiates service instances as defined by a ServiceCollection.
    /// The ServiceCollection builds the list of service descriptors and in turn can build a ServiceProvider.
    /// 
    /// For singleton services, the provider maintains a single instance for the lifetime of the provider.
    /// For scoped services, the provider can not build these and returns null. A ServiceScope is required.
    /// For transient services, the provider builds a new instance on each request.
    /// 
    /// The ServiceProvider is thread-safe. In a well-behaved application there should
    /// onyl be a single ServiceCollection and a single ServiceProvider.
    /// </summary>
    class ServiceProvider
    {
    public:

        explicit ServiceProvider(std::vector<ServiceDescriptor> descriptors);
        ~ServiceProvider();

        template<typename ServiceType>
        std::shared_ptr<ServiceType> get() noexcept
        {
            auto service = resolve(type_id<ServiceType>());

            if (!service.has_value())
            {
                return nullptr;
            }

            return std::any_cast<std::shared_ptr<ServiceType>>(service);
        }

        std::shared_ptr<ServiceScope> createScope() const noexcept;
        uint32_t size() const noexcept;

    protected:

    private:

        friend class ServiceScope;

        std::any resolve(TypeId type) noexcept;
        std::any resolveSingleton(ServiceDescriptor const& descriptor) noexcept;
        ServiceFactoryResolver createResolver() noexcept;
        ServiceDescriptor const* find(TypeId type) const noexcept;

        struct Impl;
        ImplPtr<Impl, 256> m_impl;
    };
}

#endif