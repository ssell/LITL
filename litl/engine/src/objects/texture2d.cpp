#include "litl-engine/objects/texture2d.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Texture2D::create(Authority<ObjectPool> auth, Texture2DDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
        return true;
    }

    void Texture2D::destroy(Authority<ObjectPool> auth) noexcept
    {

    }
}