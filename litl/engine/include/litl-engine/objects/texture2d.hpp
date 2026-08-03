#ifndef LITL_ENGINE_OBJECTS_TEXTURE2D_H__
#define LITL_ENGINE_OBJECTS_TEXTURE2D_H__

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"

namespace litl
{
    class ObjectPool;

    struct Texture2DDescriptor
    {
        ObjectDescriptor objectInfo{};
    };

    class Texture2D final
    {
    public:

        bool create(Authority<ObjectPool> auth, Texture2DDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

    private:

        Texture2DDescriptor m_descriptor{};
    };
}

#endif