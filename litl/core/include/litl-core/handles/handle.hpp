#ifndef LITL_CORE_HANDLE_H__
#define LITL_CORE_HANDLE_H__

#include <cstdint>

namespace litl
{
    /// <summary>
    /// An opaque handle typically used with an owning pool.
    /// </summary>
    template<typename Tag>
    struct Handle
    {
        uint32_t index{ 0u };
        uint32_t version{ 0u };

        [[nodiscard]] bool operator==(Handle const&) const noexcept = default;
        [[nodiscard]] bool isValid() const noexcept { return version != 0; }
    };
}

#endif