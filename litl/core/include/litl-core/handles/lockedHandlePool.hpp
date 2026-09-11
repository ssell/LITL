#ifndef LITL_CORE_HANDLE_LOCKED_POOL_H__
#define LITL_CORE_HANDLE_LOCKED_POOL_H__

#include <mutex>
#include "litl-core/handles/handlePool.hpp"

namespace litl
{
    /// <summary>
    /// A lockable wrapper around HandlePool.
    /// 
    /// This should be used instead of HandlePool if it is expected that the underlying 
    /// pool is accessed outside of the main thread.
    /// 
    /// Note that the lock protects the pool's internal bookkeeping, not the returned object.
    /// </summary>
    template<typename T, typename Tag>
    class LockedHandlePool
    {
    public:

        /// <summary>
        /// Create and returns a new opaque handle for the given payload.
        /// </summary>
        [[nodiscard]] Handle<Tag> create(T& payload) noexcept
        {
            std::scoped_lock lock{ m_poolMutex };
            return m_pool.create(payload);
        }

        /// <summary>
        /// Retrieves the payload tied to the provided handle.
        /// If the handle is out-of-date, or no such payload was found, then returns nullptr.
        /// </summary>
        [[nodiscard]] T* get(Handle<Tag> handle) noexcept
        {
            std::scoped_lock lock{ m_poolMutex };
            return m_pool.get(handle);
        }

        /// <summary>
        /// Destroys (invalidates) the provided handle if it is currently valid.
        /// </summary>
        bool destroy(Handle<Tag> handle) noexcept
        {
            std::scoped_lock lock{ m_poolMutex };
            return m_pool.destroy(handle);
        }

        [[nodiscard]] bool valid(Handle<Tag> handle) const noexcept
        {
            std::scoped_lock lock{ m_poolMutex };
            return m_pool.valid(handle);
        }

        void getAllHandles(std::vector<Handle<Tag>>& handles) const noexcept
        {
            std::scoped_lock lock{ m_poolMutex };
            m_pool.getAllHandles(handles);
        }

    private:

        HandlePool<T, Tag> m_pool{};
        mutable std::mutex m_poolMutex{};
    };
}

#endif