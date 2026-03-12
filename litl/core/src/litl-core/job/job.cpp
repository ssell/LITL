#include "litl-core/job/job.hpp"

namespace LITL::Core
{
    bool JobHandle::valid(uint32_t schedulerVersion) const noexcept
    {
        return (job != nullptr) && (version == job->version) && (version == schedulerVersion);
    }
}