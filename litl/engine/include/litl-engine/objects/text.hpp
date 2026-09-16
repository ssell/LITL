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

    struct TextDescriptor final
    {
        ObjectDescriptor objectInfo{};
        std::string string{};
    };

    class Text final
    {
    public:

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