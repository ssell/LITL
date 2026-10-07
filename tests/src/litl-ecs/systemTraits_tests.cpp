#include "tests.hpp"
#include "litl-ecs/system/systemTraits.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // System Execution Policy Static Tests
    // -------------------------------------------------------------------------------------

    struct SystemWithoutExecutionPolicy {};
    struct SystemWithExclusiveExecutionPolicy { static constexpr SystemExecution Execution = SystemExecution::Exclusive; };
    struct SystemWithParallelExecutionPolicy { static constexpr SystemExecution Execution = SystemExecution::Parallel; };

    static_assert(!HasSystemExecutionOverride<SystemWithoutExecutionPolicy>);
    static_assert(HasSystemExecutionOverride<SystemWithExclusiveExecutionPolicy>);
    static_assert(HasSystemExecutionOverride<SystemWithParallelExecutionPolicy>);

    static_assert(GetSystemExecutionPolicy<SystemWithoutExecutionPolicy> == SystemExecution::Parallel);
    static_assert(GetSystemExecutionPolicy<SystemWithExclusiveExecutionPolicy> == SystemExecution::Exclusive);
    static_assert(GetSystemExecutionPolicy<SystemWithParallelExecutionPolicy> == SystemExecution::Parallel);
}