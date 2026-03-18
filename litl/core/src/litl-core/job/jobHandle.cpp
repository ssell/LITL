#include <cassert>
#include "litl-core/job/jobHandle.hpp"

namespace LITL::Core
{
    namespace
    {
        static constexpr uint32_t JobIndexMask = 0b0000'0000'1111'1111'1111'1111'1111'1111;
        static constexpr uint32_t PoolShift = 24;
    }

    JobHandle::JobHandle()
        : m_packedIndex{ 0 }
    {

    }

    JobHandle::JobHandle(uint8_t poolIndex, uint32_t jobIndex)
    {
        assert(jobIndex <= JobIndexMask);
        m_packedIndex = (static_cast<uint32_t>(poolIndex) << PoolShift) | (jobIndex & JobIndexMask);
    }


    bool JobHandle::operator==(JobHandle const& other) const noexcept
    {
        return m_packedIndex == other.m_packedIndex;
    }

    uint8_t JobHandle::pool() const noexcept
    {
        return static_cast<uint8_t>(m_packedIndex >> PoolShift);
    }

    uint32_t JobHandle::job() const noexcept
    {
        return m_packedIndex & JobIndexMask;
    }
}