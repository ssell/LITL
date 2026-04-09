#ifndef LITL_CORE_SERVICES_DESCRIPTOR_H__
#define LITL_CORE_SERVICES_DESCRIPTOR_H__

#include <any>
#include <cstdint>
#include <memory>
#include <functional>

#include "litl-core/types.hpp"
#include "litl-core/services/serviceLifetime.hpp"

namespace litl
{
    /// <summary>
    /// Inner callable for the nested factory resolver.
    /// 
    /// The inner callable takes a type, and returns an instance of that type.
    /// Effectively a type-indexed lookup function.
    /// </summary>
    using ServiceFactoryResolver = std::function<std::any(TypeId)>;

    /// <summary>
    /// Outer callable for the nested factory resolver.
    /// 
    /// Takes the inner lookup callable as its argument and returns the std::any.
    /// The outer callable is a factory that isn't directly aware of its dependencies.
    /// Instead of taking in a concrete value, it instead receives a generic resolver.
    /// </summary>
    using ServiceFactoryFunc = std::function<std::any(ServiceFactoryResolver)>;

    /// <summary>
    /// Description of a dependency injected service.
    /// </summary>
    struct ServiceDescriptor
    {
        TypeId type;
        ServiceLifetime lifetime;
        ServiceFactoryFunc factory;
    };

    /// <summary>
    /// Utility function for resolving a service type into an instance.
    /// </summary>
    /// <typeparam name="ServiceType"></typeparam>
    /// <param name="resolver"></param>
    /// <returns></returns>
    template <typename ServiceType>
    std::shared_ptr<ServiceType> resolveService(ServiceFactoryResolver resolver) {
        auto result = resolver(type_id<ServiceType>());
        return std::any_cast<std::shared_ptr<ServiceType>>(result);
    }
}

#endif