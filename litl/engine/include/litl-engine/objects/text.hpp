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

    enum class TextType : uint32_t
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

        [[nodiscard]] bool create(Authority<ObjectPool> auth, TextDescriptor const& descriptor) noexcept
        {
            return create(descriptor);
        }

        [[nodiscard]] bool create(Authority<TextAsset> auth, TextDescriptor const& descriptor) noexcept
        {
            return create(descriptor);
        }

        void destroy(Authority<ObjectPool> auth) noexcept;

        [[nodiscard]] std::string_view string() const noexcept;

    private:

        [[nodiscard]] bool create(TextDescriptor const& descriptor) noexcept;

        TextDescriptor m_descriptor{};
    };
}

#endif