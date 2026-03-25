#include <vector>

#include "litl-ecs/system/system.hpp"
#include "litl-ecs/archetype/archetype.hpp"
#include "litl-ecs/archetype/archetypeRegistry.hpp"

namespace LITL::ECS
{
    /// <summary>
    /// This part is fun.
    /// 
    /// Originally there was a System::m_setupFunc and System::m_runFunc which were std::function<SystemSetupFunc> and std::function<SystemRunFunc>, respectively.
    /// However there were concerns about having std::function (particularly the run func) on such a hotpath that is running ECS systems.
    /// 
    /// Within storedSystemWrapper is a locally stored typed SystemWrapper, which is created in attach.
    /// The individual funcs call into that local SystemWrapper.
    /// 
    /// By doing this, we avoid the costs associated with std::function. Of primary concern in here is the indirect call introduced by 
    /// the used std::function (potentially removing compiler optimizations) and avoiding the heap allocation (and freeing) done to capture non-trivial state.
    /// </summary>
    struct StoredWrappedFunctions
    {
        alignas(16) std::byte storedSystemWrapper[SystemWrapperFuncStorageSize];
        ErasedSystemSetupFunc setupFunc{ nullptr };
        ErasedSystemRunFunc runFunc{ nullptr };
        ErasedSystemWrapperDestroyFunc destroyFunc{ nullptr };

        ~StoredWrappedFunctions()
        {
            if (destroyFunc != nullptr)
            {
                (*destroyFunc)(storedSystemWrapper);
            }
        }
    };

    struct System::Impl
    {
        const SystemTypeId id;
        StoredWrappedFunctions functions;
        std::vector<ComponentTypeId> componentTypes;
        std::vector<Archetype*> archetypes;
    };

    namespace
    {
        static SystemTypeId nextSystemId()
        {
            static SystemTypeId id = 0;
            return id++;
        }
    }

    System::System()
        : m_pImpl(std::make_unique<System::Impl>(nextSystemId()))
    {

    }
    
    System::~System()
    {

    }

    SystemTypeId System::id() const noexcept
    {
        return m_pImpl->id;
    }

    void System::reset() noexcept
    {
        m_pImpl->archetypes.clear();
    }


    void* System::getLocalWrapperStorageAddress()
    {
        return &m_pImpl->functions.storedSystemWrapper;
    }

    void System::storeLocalWrapperFunctions(ErasedSystemSetupFunc setup, ErasedSystemRunFunc run, ErasedSystemWrapperDestroyFunc destroy)
    {
        m_pImpl->functions.setupFunc = setup;
        m_pImpl->functions.runFunc = run;
        m_pImpl->functions.destroyFunc = destroy;
    }

    void System::registerComponentType(ComponentTypeId const componentType) const noexcept
    {
        m_pImpl->componentTypes.push_back(componentType);
    }

    void System::updateArchetypes(std::vector<ArchetypeId> const& newArchetypes) const noexcept
    {
        for (auto archetypeId : newArchetypes)
        {
            bool validArchetype = true;
            auto* archetype = ArchetypeRegistry::getById(archetypeId);
            
            for (auto i = 0; i < m_pImpl->componentTypes.size() && validArchetype; ++i)
            {
                validArchetype = validArchetype && archetype->hasComponent(m_pImpl->componentTypes[i]);
            }

            if (validArchetype)
            {
                m_pImpl->archetypes.push_back(archetype);
            }
        }
    }

    void System::setup(Core::ServiceProvider& services)
    {
        assert(m_pImpl->functions.setupFunc != nullptr);
        m_pImpl->functions.setupFunc(m_pImpl->functions.storedSystemWrapper, services);
    }

    void System::run(World& world, float dt)
    {
        assert(m_pImpl->functions.runFunc != nullptr);

        for (auto archetype : m_pImpl->archetypes)
        {
            const auto chunkCount = archetype->chunkCount();
            const auto& layout = archetype->chunkLayout();

            for (auto ci = 0; ci < chunkCount; ++ci)
            {
                m_pImpl->functions.runFunc(m_pImpl->functions.storedSystemWrapper, world, dt, archetype->getChunk(ci), layout);
            }
        }
    }

    void System::run(World& world, float dt, Core::JobScheduler& scheduler, Core::JobFence& fence)
    {
        assert(m_pImpl->functions.runFunc != nullptr);

        for (auto* archetype : m_pImpl->archetypes)
        {
            const auto chunkCount = archetype->chunkCount();

            for (auto ci = 0; ci < chunkCount; ++ci)
            {
                scheduler.createAndSubmit([this, &world, dt, archetype, ci](Core::Job* job)
                {
                    (*m_pImpl->functions.runFunc)(m_pImpl->functions.storedSystemWrapper, world, dt, archetype->getChunk(ci), archetype->chunkLayout());
                }, fence, nullptr);
            }
        }
    }
}