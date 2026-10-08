#include "tests.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // System Execution Policy Static Tests
    // -------------------------------------------------------------------------------------

    struct SystemWithoutExecutionPolicy {};
    struct SystemWithExclusiveExecutionPolicy { static constexpr SystemExecutionPolicy ExecutionPolicy = SystemExecutionPolicy::Exclusive; };
    struct SystemWithParallelExecutionPolicy { static constexpr SystemExecutionPolicy ExecutionPolicy = SystemExecutionPolicy::Parallel; };

    static_assert(!HasSystemExecutionOverride<SystemWithoutExecutionPolicy>);
    static_assert(HasSystemExecutionOverride<SystemWithExclusiveExecutionPolicy>);
    static_assert(HasSystemExecutionOverride<SystemWithParallelExecutionPolicy>);

    static_assert(GetSystemExecutionPolicy<SystemWithoutExecutionPolicy> == SystemExecutionPolicy::Parallel);
    static_assert(GetSystemExecutionPolicy<SystemWithExclusiveExecutionPolicy> == SystemExecutionPolicy::Exclusive);
    static_assert(GetSystemExecutionPolicy<SystemWithParallelExecutionPolicy> == SystemExecutionPolicy::Parallel);
}