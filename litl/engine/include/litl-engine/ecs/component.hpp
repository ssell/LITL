#ifndef LITL_ENGINE_ECS_COMPONENT_H__
#define LITL_ENGINE_ECS_COMPONENT_H__

#include <utility>

namespace LITL::Engine::ECS
{
    using ComponentTypeId = uint32_t;

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
        static ComponentDescriptor* get()
        {
            // Use a static descriptor that is different for each specialization of this template.
            // This ensures a stable pointer to the descriptor that exists for the lifetime of the application.
            static ComponentDescriptor descriptor = create<T>();
            return &descriptor;
        }

    private:

        template<typename T>
        static ComponentDescriptor create()
        {
            return {
                nextId(),
                sizeof(T),
                alignof(T),
                [](void* to) { new (to) T(); },                                                     // allocate into the pre-existing buffer location being pointed to
                [](void* from, void* to) { new (to) T(std::move(*reinterpret_cast<T*>(from))); },   // move into the other specified location
                [](void* ptr) { reinterpret_cast<T*>(ptr)->~T(); }                                  // invoke the destructor for T
            };
        }

        static ComponentTypeId nextId()
        {
            // Never assign id 0 to let it indicate an uninitialized component type id.
            static ComponentTypeId NextId = 1;
            return NextId++;
        }
    };
}

#endif