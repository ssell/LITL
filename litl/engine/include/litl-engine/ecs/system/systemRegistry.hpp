#ifndef LITL_ENGINE_ECS_SYSTEM_REGISTRY_H__
#define LITL_ENGINE_ECS_SYSTEM_REGISTRY_H__

#include <memory>

#include "litl-core/debug.hpp"
#include "litl-engine/ecs/system/systemRunner.hpp"

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

        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {

        }

        const SystemTypeId id;

    private:

        static SystemTypeId nextId() noexcept;
        static SystemTypeId track(SystemRecord const* record) noexcept;

        struct Internal 
        { 
            
        };

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
            Impl(System sys) 
                : system(std::move(sys)), runner(system)
            {

            }

            System system;
            SystemRunner<System> runner;
        };

        std::unique_ptr<Internal> m_pInternal;
    };



    template<ValidSystem System>
    class SystemRecord2
    {
    public:

        SystemRecord2(SystemTypeId id)
            : id(id), system({}), runner({ system })
        {

        }

        void run(World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {
            runner.run(world, dt, chunk, layout);
        }

        const SystemTypeId id;

    private:

        System system;
        SystemRunner<System> runner;
    };



    class SystemRegistry
    {
    public:

        static SystemTypeId nextId()
        {
            static SystemTypeId id = 0;
            return id++;
        }

        template<ValidSystem System>
        static SystemRecord2<System>& get2()
        {
            static SystemRecord2<System> record({ nextId() });
            return record;
        }

        static void run(SystemTypeId, World& world, float dt, Chunk& chunk, ChunkLayout const& layout)
        {

        }

        /*

        template<ValidSystem System>
        static SystemRecord const& get()
        {
            static SystemRecord record(System{});
            return record;
        }

        static SystemRecord const& get(SystemTypeId systemTypeId);
        */
    protected:

    private:



    };
}

#endif