#include "litl-engine/objects/text.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Text::create(TextDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
        return true;
    }

    void Text::destroy(Authority<ObjectPool> auth) noexcept
    {

    }

    std::string_view Text::string() const noexcept
    {
        return m_descriptor.string;
    }

    TextType Text::type() const noexcept
    {
        return m_descriptor.type;
    }
}