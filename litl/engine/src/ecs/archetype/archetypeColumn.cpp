#include <new>
#include "litl-engine/ecs/archetype/archetypeColumn.hpp"

namespace LITL::Engine::ECS
{
    struct ArchetypeColumn::Impl
    {
        /// <summary>
        /// Contains component size, alignment, and operation function pointers.
        /// </summary>
        ComponentDescriptor const* descriptor = nullptr;

        /// <summary>
        /// Number of components stored in the buffer.
        /// </summary>
        size_t count = 0;

        /// <summary>
        /// Number of components that can be stored in the buffer.
        /// </summary>
        size_t capacity = 0;

        /// <summary>
        /// Data buffer.
        /// </summary>
        uint8_t* data = nullptr;
    };

    ArchetypeColumn::ArchetypeColumn(ComponentDescriptor const* pDescriptor, size_t capacity)
    {
        m_impl->descriptor = pDescriptor;
        reserve(capacity);
    }

    void ArchetypeColumn::reserve(size_t newCapacity)
    {
        if (newCapacity == 0)
        {
            return;
        }

        // Allocate new storage buffer. Using `::operator new` to allocate uninitialized memory (`new` allocates and initializes)
        uint8_t* newData = reinterpret_cast<uint8_t*>(::operator new(
            newCapacity * m_impl->descriptor->size,
            std::align_val_t(m_impl->descriptor->alignment)));

        // Move and delete existing
        if (m_impl->data != nullptr)
        {
            for (size_t i = 0; i < m_impl->count; ++i)
            {
                void* from = m_impl->data + (i * m_impl->descriptor->size);
                void* to = newData + (i * m_impl->descriptor->size);

                m_impl->descriptor->move(from, to);
                m_impl->descriptor->destroy(from);
            }

            ::operator delete(m_impl->data, std::align_val_t(m_impl->descriptor->alignment));
        }

        m_impl->data = newData;
        m_impl->capacity = newCapacity;
    }

    void ArchetypeColumn::add()
    {
        if (m_impl->count == m_impl->capacity)
        {
            reserve(m_impl->capacity ? m_impl->capacity * 2 : 8);
        }

        void* target = m_impl->data + (m_impl->count * m_impl->descriptor->size);
        m_impl->descriptor->build(target);
        m_impl->count++;
    }

    void ArchetypeColumn::remove(size_t index)
    {
        // This performs a destroy+swap to keep the back index open without having to shift contents.
        const size_t back = m_impl->count - 1;

        void* targetPtr = m_impl->data + (index * m_impl->descriptor->size);
        void* backPtr = m_impl->data + (back * m_impl->descriptor->size);

        m_impl->descriptor->destroy(targetPtr);

        if (index != back)
        {
            // Move what is at the back to the newly opened index, and then clear out the back.
            m_impl->descriptor->move(backPtr, targetPtr);
            m_impl->descriptor->destroy(backPtr);
        }

        m_impl->count--;
    }

    void* ArchetypeColumn::get(size_t const index)
    {
        return (m_impl->data + (index * m_impl->descriptor->size));
    }

    size_t ArchetypeColumn::count() const noexcept
    {
        return m_impl->count;
    }

    size_t ArchetypeColumn::capacity() const noexcept
    {
        return m_impl->capacity;
    }
}