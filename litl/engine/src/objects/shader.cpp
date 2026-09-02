#include "litl-engine/objects/shader.hpp"
#include "litl-engine/objects/objectPool.hpp"

namespace litl
{
    bool Shader::create(Authority<ObjectPool> auth, ShaderDescriptor const& descriptor) noexcept
    {
        m_descriptor = descriptor;

        // ... todo ...
        
        return true;
    }

    bool Shader::create(Authority<ObjectPool> auth, ObjectDescriptor const& descriptor) noexcept
    {
        m_descriptor.objectInfo = descriptor;
        return true;
    }

    void Shader::destroy(Authority<ObjectPool> auth) noexcept
    {
        // ... todo ...
    }
}