#include "litl-engine/objects/text.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Text::create(Authority<ObjectPool> auth, TextDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
        return true;
    }

    void Text::destroy(Authority<ObjectPool> auth) noexcept
    {

    }
}