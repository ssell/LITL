#ifndef LITL_CORE_SERVICES_COLLECTION_H__
#define LITL_CORE_SERVICES_COLLECTION_H__

#include <memory>
#include <typeindex>
#include <vector>

#include "litl-core/types.hpp"
#include "litl-core/traits.hpp"
#include "litl-core/services/serviceDescriptor.hpp"

namespace LITL::Core
{
    class ServiceProvider;

    template<typename ServiceType>
    using ServiceFactory = std::function<std::shared_ptr<ServiceType>(ServiceFactoryResolver)>;

    class ServiceCollection
    {
    public:

        /// <summary>
        /// Registers a factory function for the service type with a specified lifetime.
        /// 
        /// Takes in a factory function that returns a shared pointer to an instance of the service.
        /// The factory function itself takes in a dependency resolver.
        /// </summary>
        /// <typeparam name="ServiceType"></typeparam>
        /// <param name="lifetime"></param>
        /// <returns></returns>
        template<typename ServiceType, typename ServiceImpl> requires InheritsFrom<ServiceType, ServiceImpl>
        ServiceCollection& add(ServiceLifetime lifetime, ServiceFactory<ServiceType> factory)
        {
            m_descriptors.push_back(ServiceDescriptor{
                .type = type_id<ServiceType>(),
                .lifetime = lifetime,
                .factory = [factory = std::move(factory)](ServiceFactoryResolver resolver) -> std::any
                    {
                        return factory(std::move(resolver));
                    }
                });

            return *this;
        }

        /// <summary>
        /// Adds a singleton service.
        /// User provided factory function for when the singleton has a non-trivial construction.
        /// </summary>
        /// <typeparam name="ServiceType"></typeparam>
        /// <returns></returns>
        template<typename ServiceType, typename ServiceImpl> requires InheritsFrom<ServiceType, ServiceImpl>
        ServiceCollection& addSingleton(ServiceFactory<ServiceType> factory)
        {
            return add<ServiceType, ServiceImpl>(ServiceLifetime::Singleton, std::move(factory));
        }

        /// <summary>
        /// Adds a singleton service that has no dependencies.
        /// </summary>
        /// <typeparam name="ServiceType"></typeparam>
        /// <returns></returns>
        template<typename ServiceType, typename ServiceImpl> requires InheritsFrom<ServiceType, ServiceImpl>
        ServiceCollection& addSingleton()
        {
            return addSingleton<ServiceType, ServiceImpl>([](auto resolver) { return std::make_shared<ServiceImpl>(); });
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        std::shared_ptr<ServiceProvider> build() const;

        /// <summary>
        /// Returns the number of descriptors stored in the service collection.
        /// </summary>
        /// <returns></returns>
        size_t size() const noexcept;

    protected:

    private:

        std::vector<ServiceDescriptor> m_descriptors;
    };
}

#endif