#ifndef LITL_CORE_ID_H__
#define LITL_CORE_ID_H__

#include <atomic>
#include <cstdint>
#include <format>
#include <string>

namespace litl
{
    /// <summary>
    /// Given a prefix, generates an unique string id/name.
    /// The templating is to ensure an unique count for each specified type.
    /// For example:
    /// 
    ///     generateId<TextureResource>("Texture");         // "Texture_1"
    ///     generateId<TextureResource>("Texture");         // "Texture_2"
    ///     generateId<BufferResource>("Buffer");           // "Buffer_1"
    ///     generateId<TextureResource>("Texture");         // "Texture_3"
    ///     generateId<BufferResource>("Buffer");           // "Buffer_2"
    /// </summary>
    template<typename T>
    [[nodiscard]] std::string generateId(std::string_view prefix) noexcept
    {
        static std::atomic<uint64_t> idCounter{ 1ull };
        return std::format("{}_{}", prefix, idCounter.fetch_add(1, std::memory_order_relaxed));
    }
}

#endif