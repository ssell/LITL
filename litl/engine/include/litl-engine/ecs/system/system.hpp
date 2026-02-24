#ifndef LITL_ENGINE_ECS_SYSTEM_H__
#define LITL_ENGINE_ECS_SYSTEM_H__

#include <concepts>

namespace LITL::Engine::ECS
{
    template<typename System>
    concept ValidSystem = requires { typename System::SystemComponents; };

    /// <summary>
    /// Used to define the components that a System operates on.
    /// </summary>
    /// <typeparam name="...SystemComponents"></typeparam>
    template<typename... SystemComponents>
    struct SystemComponentList 
    {
    
    };

    /// <summary>
    /// Traits adapter used to extract the components.
    /// </summary>
    /// <typeparam name="System"></typeparam>
    template<ValidSystem System>
    struct SystemTraits
    {
        using SystemComponents = typename System::SystemComponents;
    };

    template<typename ComponentList>
    struct ExpandComponentList;

    template<typename... Components>
    struct ExpandComponentList<SystemComponentList<Components...>>
    {
        template<typename Fn>
        static void apply(Fn&& fn)
        {
            fn.template operator()<Components...>();
        }
    };
}

#endif