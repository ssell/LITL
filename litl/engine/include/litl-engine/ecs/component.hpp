#ifndef LITL_ENGINE_ECS_COMPONENT_H__
#define LITL_ENGINE_ECS_COMPONENT_H__

#include <string_view>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "litl-engine/ecs/constants.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Components are stored in raw byte buffers.
    /// These descriptors define the shape and how to interact with them.
    /// </summary>
    struct ComponentDescriptor
    {
        /// <summary>
        /// Component ID that is stable/guaranteed for the current application run only. Use for indexing, etc.
        /// </summary>
        ComponentTypeId id;

        /// <summary>
        /// Component ID that is stable/guaranteed across application runs. Use for networking, serialization, etc.
        /// </summary>
        StableComponentTypeId stableId;

        size_t size;
        size_t alignment;

        void (*build)(void* destination);
        void (*move)(void* from, void* to);
        void (*destroy)(void* ptr);

        template<typename T>
        static ComponentDescriptor const* get() noexcept
        {
            static_assert(std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>);

            // Use a static descriptor that is different for each specialization of this template.
            // This ensures a stable pointer to the descriptor that exists for the lifetime of the application.
            static ComponentDescriptor descriptor = {
                .id        = nextId(),
                .stableId  = getStableId<T>(),
                .size      = sizeof(T),
                .alignment = alignof(T),
                .build     = [](void* to) { new (to) T(); },                                                     // allocate into the pre-existing buffer location being pointed to
                .move      = [](void* from, void* to) { new (to) T(std::move(*reinterpret_cast<T*>(from))); },   // move into the other specified location
                .destroy   = [](void* ptr) { reinterpret_cast<T*>(ptr)->~T(); }                                  // invoke the destructor for T
            };

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
}

#endif