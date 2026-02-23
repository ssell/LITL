#ifndef LITL_ENGINE_ECS_COMPONENT_H__
#define LITL_ENGINE_ECS_COMPONENT_H__

#include <functional>
#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

#include "litl-core/debug.hpp"
#include "litl-engine/ecs/constants.hpp"

namespace LITL::Engine::ECS
{
    using ComponentBuildFunc   = void (*)(void* destination);
    using ComponentMoveFunc    = void (*)(void* from, void* to);
    using ComponentDestroyFunc = void (*)(void* ptr);

    /// <summary>
    /// Components are stored in raw byte buffers.
    /// These descriptors define the shape and how to interact with them.
    /// </summary>
    struct ComponentDescriptor : public DebugInfo
    {
        ComponentDescriptor(
            std::string_view name, 
            ComponentTypeId id, 
            StableComponentTypeId stableId, 
            size_t size, 
            size_t alignment,
            ComponentBuildFunc build,
            ComponentMoveFunc move,
            ComponentDestroyFunc destroy)
            : id(id), stableId(stableId), size(size), alignment(alignment), build(build), move(move), destroy(destroy)
        {
            setDebugName(name);
        }

        /// <summary>
        /// Component ID that is stable/guaranteed for the current application run only. Use for indexing, etc.
        /// </summary>
        const ComponentTypeId id;

        /// <summary>
        /// Component ID that is stable/guaranteed across application runs. Use for networking, serialization, etc.
        /// </summary>
        const StableComponentTypeId stableId;

        const size_t size;
        const size_t alignment;

        const ComponentBuildFunc build;
        const ComponentMoveFunc move;
        const ComponentDestroyFunc destroy;

        template<typename T>
        static ComponentDescriptor const* get() noexcept
        {
            static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>);

            // Use a static descriptor that is different for each specialization of this template.
            // This ensures a stable pointer to the descriptor that exists for the lifetime of the application.
            static ComponentDescriptor descriptor(
                typeid(T).name(),
                nextId(),
                getStableId<T>(),
                sizeof(T),
                alignof(T),
                [](void* to) { new (to) T(); },                                                     // allocate into the pre-existing buffer location being pointed to
                [](void* from, void* to) { new (to) T(std::move(*reinterpret_cast<T*>(from))); },   // move into the other specified location
                [](void* ptr) { reinterpret_cast<T*>(ptr)->~T(); });                                // invoke the destructor for T 

            track(&descriptor);

            return &descriptor;
        }

        static ComponentDescriptor const* get(ComponentTypeId componentTypeId) noexcept;

        template<typename T>
        static StableComponentTypeId getStableId() noexcept
        {
            return getStableId(typeid(T).name());
        }

        static StableComponentTypeId getStableId(std::string_view componentName) noexcept;

    private:

        static ComponentTypeId nextId() noexcept;
        static void track(ComponentDescriptor const* descriptor);
    };

    template<typename T>
    ComponentTypeId getComponentTypeId() noexcept
    {
        return ComponentDescriptor::get<T>()->id;
    }

    template<typename T>
    StableComponentTypeId getStableComponentTypeId() noexcept
    {
        return ComponentDescriptor::get<T>()->stableId;
    }

    template<typename ComponentType>
    void foldComponentTypesIntoVector(std::vector<ComponentTypeId>& componentTypeIds) noexcept
    {
        componentTypeIds.emplace_back(ComponentDescriptor::get<ComponentType>()->id);
    }
}

#endif