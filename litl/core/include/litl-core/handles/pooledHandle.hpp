#ifndef LITL_CORE_POOLED_HANDLE_H__
#define LITL_CORE_POOLED_HANDLE_H__

#include "litl-core/containers/pagedVector.hpp"

namespace litl
{
    /// <summary>
    /// A pooled opaque handle.
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    template<typename Tag>
    struct Handle
    {
        uint32_t index;
        uint32_t version;

        bool operator==(Handle const&) const noexcept = default;
        bool isValid() const noexcept { return version != 0; }
    };

    /// <summary>
    /// A generic handle pool.
    /// 
    /// Used by various backends to manage their opaque handles.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <typeparam name="Tag"></typeparam>
    template<typename T, typename Tag>
    class HandlePool
    {
    public:

        /// <summary>
        /// Create and returns a new opaque handle for the given payload.
        /// </summary>
        /// <param name="payload"></param>
        /// <returns></returns>
        [[nodiscard]] Handle<Tag> create(T payload) noexcept
        {
            uint32_t index = 0;

            if (!m_freeList.empty())
            {
                index = m_freeList.back();
                m_freeList.pop_back();
            }
            else
            {
                index = static_cast<uint32_t>(m_slots.size());
                m_slots.emplace_back();
            }

            m_slots[index].version++;
            m_slots[index].payload = std::move(payload);
            m_slots[index].set = true;

            return Handle<Tag>{ index, m_slots[index].version };
        }

        /// <summary>
        /// Retrieves the payload tied to the provided handle.
        /// If the handle is out-of-date, or no such payload was found, then returns nullptr.
        /// </summary>
        /// <param name="handle"></param>
        /// <returns></returns>
        [[nodiscard]] T* get(Handle<Tag> handle) noexcept
        {
            if (!valid(handle))
            {
                return nullptr;
            }

            return &m_slots[handle.index].payload;
        }

        /// <summary>
        /// Destroys (invalidates) the provided handle if it is currently valid.
        /// </summary>
        /// <param name="handle"></param>
        bool destroy(Handle<Tag> handle) noexcept
        {
            if (!valid(handle))
            {
                return false;
            }

            m_slots[handle.index].version++;    // invalidate the the old handle by bumping the version
            m_slots[handle.index].payload = T{};
            m_slots[handle.index].set = false;
            m_freeList.push_back(handle.index);

            return true;
        }

        [[nodiscard]] bool valid(Handle<Tag> handle) const noexcept
        {
            return
                (handle.index < m_slots.size()) &&
                (handle.version == m_slots[handle.index].version) &&
                (handle.version > 0);
        }

        void getAllHandles(std::vector<Handle<Tag>>& handles) const noexcept
        {
            handles.reserve(m_slots.size());

            for (auto i = 0u; i < static_cast<uint32_t>(m_slots.size()); ++i)
            {
                if (m_slots[i].set == true)
                {
                    handles.push_back(Handle<Tag>{ i, m_slots[i].version });
                }
            }
        }

    private:

        struct Slot
        {
            uint32_t version;
            T payload;
            bool set = false;
        };

        PagedVector<Slot, 32> m_slots;
        std::vector<uint32_t> m_freeList;
    };
}

#endif