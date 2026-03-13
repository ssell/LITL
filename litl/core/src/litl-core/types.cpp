#include "litl-core/types.hpp"

namespace LITL
{
    TypeId next_type_id__() noexcept
    {
        static std::atomic<TypeId> next{ 0 };
        return next.fetch_add(1, std::memory_order_relaxed);
    }
}