#ifndef LITL_ENGINE_OBJECTS_TEXT_H__
#define LITL_ENGINE_OBJECTS_TEXT_H__

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"

namespace litl
{
    class ObjectPool;

    struct TextDescriptor
    {
        ObjectDescriptor objectInfo{};
    };

    class Text final
    {
    public:

        bool create(Authority<ObjectPool> auth, TextDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

    private:

        TextDescriptor m_descriptor{};
    };
}

#endif