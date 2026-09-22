#include "litl-engine/objects/texture.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Texture::create(Authority<ObjectPool> auth, TextureDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
        return true;
    }

    void Texture::destroy(Authority<ObjectPool> auth) noexcept
    {

    }
}