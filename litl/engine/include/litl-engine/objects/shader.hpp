#ifndef LITL_ENGINE_OBJECTS_SHADER_H__
#define LITL_ENGINE_OBJECTS_SHADER_H__

#include <memory>
#include <span>

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class ObjectPool;

    struct ShaderDescriptor
    {
        ObjectDescriptor objectInfo{};
    };

    class Shader
    {
    public:

        [[nodiscard]] bool create(Authority<ObjectPool> auth, ShaderDescriptor const& descriptor) noexcept;
        [[nodiscard]] bool create(Authority<ObjectPool> auth, ObjectDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

    private:

        ShaderDescriptor m_descriptor;
    };
}

#endif