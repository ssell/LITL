#ifndef LITL_ENGINE_ECS_ARCHETYPE_COLUMN_H__
#define LITL_ENGINE_ECS_ARCHETYPE_COLUMN_H__

#include <cstdint>

#include "litl-core/impl.hpp"
#include "litl-engine/ecs/component.hpp"

namespace LITL::Engine::ECS
{
    /// <summary>
    /// Storage for a singular component type in an Archetype.
    /// The component is described (and managed) by its ComponentDescriptor.
    /// </summary>
    class ArchetypeColumn
    {
    public:

        /// <summary>
        /// If capacity is zero then it is lazily allocated at time of first call to add.
        /// </summary>
        /// <param name="pDescriptor"></param>
        /// <param name="capacity"></param>
        ArchetypeColumn(ComponentDescriptor const* pDescriptor, size_t capacity);
        ArchetypeColumn(ArchetypeColumn const&) = delete;
        ArchetypeColumn& operator=(ArchetypeColumn const&) = delete;

        /// <summary>
        /// Increases the size of the internal buffer to specified size.
        /// </summary>
        /// <param name="newCapacity"></param>
        void reserve(size_t newCapacity);

        /// <summary>
        /// Adds another component instance to the buffer.
        /// If the buffer is at capacity, the capacity is increased first.
        /// </summary>
        void add();

        /// <summary>
        /// Places the component at the specified source into this column.
        /// </summary>
        /// <param name="source"></param>
        void move(void* from);

        /// <summary>
        /// Removes the component at the specified index.
        /// The caller assumes the responsibility of providing a valid index.
        /// </summary>
        /// <param name="index"></param>
        void remove(size_t index);

        /// <summary>
        /// Returns a pointer to the component at the specified index.
        /// The caller assumes the responsibility of providing a valid index.
        /// </summary>
        /// <param name="index"></param>
        /// <returns></returns>
        void* get(size_t index);

        /// <summary>
        /// Returns a pointer directly to the inner buffer.
        /// </summary>
        /// <returns></returns>
        void* data();

        /// <summary>
        /// Returns the type of components stored in this column.
        /// </summary>
        /// <returns></returns>
        ComponentTypeId componentType() const noexcept;

        /// <summary>
        /// Returns the number of components stored in the buffer.
        /// </summary>
        /// <returns></returns>
        size_t count() const noexcept;

        /// <summary>
        /// Returns the number of components the underlying buffer can contain.
        /// </summary>
        /// <returns></returns>
        size_t capacity() const noexcept;

        /// <summary>
        /// Returns the component descriptor for this column.
        /// </summary>
        /// <returns></returns>
        ComponentDescriptor const* descriptor() const noexcept;

    protected:

    private:

        void resize();

        struct Impl;
        Core::ImplPtr<Impl, 32> m_impl;
    };
}

#endif