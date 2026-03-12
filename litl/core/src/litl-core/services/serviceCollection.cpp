#include <memory>

#include "litl-core/services/serviceCollection.hpp"
#include "litl-core/services/serviceProvider.hpp"

namespace LITL::Core
{
    std::shared_ptr<ServiceProvider> ServiceCollection::build() const
    {
        return std::make_shared<ServiceProvider>(m_descriptors);
    }

    size_t ServiceCollection::size() const noexcept
    {
        return m_descriptors.size();
    }
}