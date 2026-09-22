#ifndef LITL_ENGINE_OBJECTS_TEXTURE_H__
#define LITL_ENGINE_OBJECTS_TEXTURE_H__

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"

namespace litl
{
    class ObjectPool;

    struct TextureDescriptor
    {
        ObjectDescriptor objectInfo{};
    };

    class Texture final
    {
    public:

        [[nodiscard]] bool create(Authority<ObjectPool> auth, TextureDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

    private:

        TextureDescriptor m_descriptor{};
    };
}

#endif