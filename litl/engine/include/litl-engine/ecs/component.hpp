#ifndef LITL_ENGINE_ECS_COMPONENT_H__
#define LITL_ENGINE_ECS_COMPONENT_H__

#include <type_traits>
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
        ComponentTypeId id;

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
                .size      = sizeof(T),
                .alignment = alignof(T),
                .build     = [](void* to) { new (to) T(); },                                                     // allocate into the pre-existing buffer location being pointed to
                .move      = [](void* from, void* to) { new (to) T(std::move(*reinterpret_cast<T*>(from))); },   // move into the other specified location
                .destroy   = [](void* ptr) { reinterpret_cast<T*>(ptr)->~T(); }                                  // invoke the destructor for T
            };

            return &descriptor;
        }

        static ComponentDescriptor const* get(ComponentTypeId componentTypeId) noexcept;

    private:

        static ComponentTypeId nextId() noexcept;
    };

    template<typename T>
    ComponentTypeId getComponentTypeId() noexcept
    {
        return ComponentDescriptor::get<T>()->id;
    }
}

#endif