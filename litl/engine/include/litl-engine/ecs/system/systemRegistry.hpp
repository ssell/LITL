#ifndef LITL_ENGINE_ECS_SYSTEM_REGISTRY_H__
#define LITL_ENGINE_ECS_SYSTEM_REGISTRY_H__

#include <memory>

#include "litl-core/debug.hpp"
#include "litl-engine/ecs/system/system.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Wrapper around a System instance pointer and an application-unique id for it.
    /// </summary>
    class SystemRecord : DebugInfo
    {
    public:

        template<ValidSystem System>
        SystemRecord(System system)
            : id(track(this)), m_pInternal(std::make_unique<Impl<System>>(std::move(system)))
        {
            setDebugName(typeid(System).name());
        }

        const SystemTypeId id;

    private:

        static SystemTypeId nextId() noexcept;
        static SystemTypeId track(SystemRecord const* record) noexcept;

        struct Internal { };

        /// <summary>
        /// In order to pass the SystemRecord around generically (so we
        /// can use "SystemRecord" instead of "SystemRecord<T>") we store
        /// the system pointer within this inner templated struct.
        /// 
        /// This inner struct itself is effectively wrapped in an unique_ptr
        /// to hide away it's templated type as well at runtime.
        /// </summary>
        /// <typeparam name="System"></typeparam>
        template<ValidSystem System>
        struct Impl : Internal
        {
            Impl(System sys) : system(std::move(sys)), runner(system)
            {

            }

            System system;
            SystemRunner<System> runner;
        };

        std::unique_ptr<Internal> m_pInternal;
    };

    class SystemRegistry
    {
    public:

        template<ValidSystem System>
        static SystemRecord const& get()
        {
            static SystemRecord record(System{});
            return record;
        }

        static SystemRecord const& get(SystemTypeId systemTypeId);

    protected:

    private:

    };
}

#endif