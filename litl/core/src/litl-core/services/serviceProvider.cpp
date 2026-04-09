#include <any>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "litl-core/services/serviceProvider.hpp"
#include "litl-core/services/serviceScope.hpp"

namespace litl
{
    struct ServiceProvider::Impl
    {
        std::vector<ServiceDescriptor> descriptors;
        std::unordered_map<TypeId, std::any> singletonServices;
        std::mutex singletonMutex;
    };

    ServiceProvider::ServiceProvider(std::vector<ServiceDescriptor> descriptors)
    {
        m_impl->descriptors.reserve(descriptors.size());

        // Store the descriptors in reverse order. If descriptors for duplicate types are added, then the later descriptor wins out.
        for (auto iter = descriptors.rbegin(); iter != descriptors.rend(); ++iter)
        {
            m_impl->descriptors.push_back((*iter));
        }

        m_impl->singletonServices.reserve(m_impl->descriptors.size());
    }

    ServiceProvider::~ServiceProvider()
    {

    }

    std::shared_ptr<ServiceScope> ServiceProvider::createScope() const noexcept
    {
        return std::make_shared<ServiceScope>(const_cast<ServiceProvider*>(this));
    }

    uint32_t ServiceProvider::size() const noexcept
    {
        return m_impl->descriptors.size();
    }

    std::any ServiceProvider::resolve(TypeId type) noexcept
    {
        auto descriptor = find(type);

        if (descriptor == nullptr)
        {
            return {};
        }

        switch (descriptor->lifetime)
        {
        case ServiceLifetime::Singleton:
            return resolveSingleton(*descriptor);

        case ServiceLifetime::Scoped:
            return {};  // must be created by a ServiceScope

        case ServiceLifetime::Transient:
            return descriptor->factory(createResolver());
        }

        return {};
    }

    ServiceDescriptor const* ServiceProvider::find(TypeId type) const noexcept
    {
        for (auto& descriptor : m_impl->descriptors)
        {
            if (descriptor.type == type)
            {
                return &descriptor;
            }
        }

        return nullptr;
    }

    bool ServiceProvider::setSingleton__(TypeId type, std::any service) noexcept
    {
        auto* descriptor = find(type);

        if (descriptor == nullptr)
        {
            return false;
        }

        if (descriptor->lifetime != ServiceLifetime::Singleton)
        {
            return false;
        }

        m_impl->singletonServices[type] = service;

        return true;
    }

    std::any ServiceProvider::resolveSingleton(ServiceDescriptor const& descriptor) noexcept
    {
        // Check if the singleton already exists
        {
            std::lock_guard<std::mutex> lock(m_impl->singletonMutex);
            auto findSingleton = m_impl->singletonServices.find(descriptor.type);

            if (findSingleton != m_impl->singletonServices.end())
            {
                return findSingleton->second;
            }
        }

        // If not, create a new instance of it and then cache it.
        auto singletonInstance = descriptor.factory(createResolver());

        // Check to make sure another thread has not already created the singleton.
        // If not, add it and return out.
        {
            std::lock_guard<std::mutex> lock(m_impl->singletonMutex);
            auto findSingleton = m_impl->singletonServices.find(descriptor.type);

            if (findSingleton != m_impl->singletonServices.end())
            {
                return findSingleton->second;
            }

            m_impl->singletonServices[descriptor.type] = singletonInstance;

            return singletonInstance;
        }
    }

    ServiceFactoryResolver ServiceProvider::createResolver() noexcept
    {
        // Utility wrapper around the lambda below.
        return [this](TypeId key) -> std::any { return this->resolve(key); };
    }
}