#ifndef LITL_CORE_JOB_HANDLE__H__
#define LITL_CORE_JOB_HANDLE__H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// Handle to a job instance. 
    /// 
    /// This should be used instead of a raw Job pointer as the JobPool recycles jobs when they are released. 
    /// A raw Job pointer may be pointing to an invalidated/out-of-date job instance and the user may not even know it.
    /// 
    /// The handle is an opaque reference, stored as a packed index into the JobPool.
    /// 
    ///     [PPPP'PPPP][IIII'IIII'IIII'IIII'IIII'IIII]
    /// 
    /// Where,
    /// 
    ///     P = Pool index
    ///     I = Job index in the pool
    /// 
    /// Packed index of 0 is reserved for NULL.
    /// 
    /// A packed index of 32-bits is used to ensure atomicity on essentially all modern platforms.
    /// </summary>
    struct JobHandle
    {
        JobHandle();
        JobHandle(uint8_t poolIndex, uint32_t jobIndex);

        bool operator==(JobHandle const& other) const noexcept;

        uint8_t pool() const noexcept;
        uint32_t job() const noexcept;

        constexpr bool isNull() const noexcept
        {
            return m_packedIndex == 0;
        }

    private:

        uint32_t m_packedIndex;
    };
}

#endif