#ifndef LITL_CORE_SERVICES_LIFETIME_H__
#define LITL_CORE_SERVICES_LIFETIME_H__

namespace LITL::Core
{
    enum class ServiceLifetime
    {
        /// <summary>
        /// One single instance, shared across the entire application.
        /// </summary>
        Singleton = 0,

        /// <summary>
        /// Created once per scope.
        /// </summary>
        Scoped = 1,

        /// <summary>
        /// Created every time it is requested.
        /// </summary>
        Transient = 2
    };
}

#endif