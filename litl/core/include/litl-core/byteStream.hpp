#ifndef LITL_CORE_BYTE_STREAM_H__
#define LITL_CORE_BYTE_STREAM_H__

#include <cstdint>
#include <span>
#include <streambuf>

namespace litl
{
    /// <summary>
    /// C++23 provides a std::ispanstream, however we are on C++20.
    /// So this is a non-owning stream buffer wrapping a contiguous memory block that can be used with std::istream, etc.
    /// </summary>
    struct ByteSpanToStreamMediator : std::streambuf
    {
        ByteSpanToStreamMediator(std::span<std::byte const> bytes)
        {
            auto* data = const_cast<char*>(reinterpret_cast<char const*>(bytes.data()));
            this->setg(data, data, data + bytes.size());    // See buffer pointers: get-start, get-next, and get-end.
        }
    };
}

#endif