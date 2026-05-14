#ifndef LITL_CORE_POOLED_HANDLE_H__
#define LITL_CORE_POOLED_HANDLE_H__

#include <cstdint>
#include <vector>

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
        void destroy(Handle<Tag> handle) noexcept
        {
            if (!valid(handle))
            {
                return;
            }

            m_slots[handle.index].version++;    // invalidate the the old handle by bumping the version
            m_slots[handle.index].payload = T{};
            m_freeList.push_back(handle.index);
        }

        [[nodiscard]] bool valid(Handle<Tag> handle) const noexcept
        {
            return
                (handle.index < m_slots.size()) &&
                (handle.version == m_slots[handle.index].version) &&
                (handle.version > 0);
        }

    private:

        struct Slot
        {
            uint32_t version;
            T payload;
        };

        std::vector<Slot> m_slots;
        std::vector<uint32_t> m_freeList;
    };
}

#endif