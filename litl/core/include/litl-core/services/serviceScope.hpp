#ifndef LITL_CORE_SERVICES_SCOPE_H__
#define LITL_CORE_SERVICES_SCOPE_H__

#include <any>
#include <memory>

#include "litl-core/impl.hpp"
#include "litl-core/types.hpp"

namespace litl
{
    class ServiceProvider;

    /// <summary>
    /// Effectively a scoped ServiceProvider.
    /// 
    /// For singletons, returns the same singleton as the creator provider does.
    /// For scoped services, the scope maintains a single instance for the lifetime of the scope.
    /// For transient services, the scope builds a new instance on each request.
    /// 
    /// A ServiceScope is not thread-safe. It is intended to be used for the duration
    /// of a single scope on a single thread. Create multiple ServiceScopes if multiple
    /// threads are being used.
    /// </summary>
    class ServiceScope
    {
    public:

        explicit ServiceScope(ServiceProvider* provider);
        ~ServiceScope();

        template<typename ServiceType>
        std::shared_ptr<ServiceType> get()
        {
            auto service = resolve(type_id<ServiceType>());

            if (!service.has_value())
            {
                return nullptr;
            }

            return std::any_cast<std::shared_ptr<ServiceType>>(service);
        }

    protected:

    private:

        std::any resolve(TypeId type) noexcept;

        struct Impl;
        ImplPtr<Impl, 128> m_impl;
    };
}

#endif