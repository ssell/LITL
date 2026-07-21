#include "litl-engine/objects/material.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Material::create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;
        
        // ... todo ...

        return true;
    }

    void Material::destroy(Authority<ObjectPool> auth) noexcept
    {
        // ... todo ...
    }
}