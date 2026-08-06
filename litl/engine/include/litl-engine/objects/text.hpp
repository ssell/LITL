#ifndef LITL_ENGINE_OBJECTS_TEXT_H__
#define LITL_ENGINE_OBJECTS_TEXT_H__

#include <concepts>
#include <string>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"

namespace litl
{
    class ObjectPool;
    class TextAsset;

    enum class TextType : uint8_t
    {
        Unknown = 0u,
        Plain = 1u,
        Json = 2u
    };

    struct TextDescriptor final
    {
        ObjectDescriptor objectInfo{};
        std::string string{};
        TextType type{ TextType::Unknown };
    };

    class Text final
    {
    public:

        template<typename T> requires std::same_as<T, ObjectPool> || std::same_as<T, TextAsset>
        [[nodiscard]] bool create(Authority<T> auth, TextDescriptor const& descriptor) noexcept
        {
            return create(descriptor);
        }

        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] std::string_view string() const noexcept;
        [[nodiscard]] TextType type() const noexcept;

    private:

        [[nodiscard]] bool create(TextDescriptor const& descriptor) noexcept;

        TextDescriptor m_descriptor{};
    };
}

#endif