#ifndef LITL_ENGINE_OBJECTS_TEXTURE_H__
#define LITL_ENGINE_OBJECTS_TEXTURE_H__

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-renderer/resources/texture.hpp"

namespace litl
{
    class ObjectPool;

    struct TextureDescriptor
    {
        ObjectDescriptor objectInfo{};
        TextureResourceDescriptor textureInfo{};
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