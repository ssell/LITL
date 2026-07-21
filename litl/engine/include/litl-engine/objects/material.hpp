#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include "litl-core/authority.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"

namespace litl
{
    class ObjectPool;

    struct MaterialDescriptor
    {
        // ... todo ...
    };

    class Material
    {
    public:

        bool create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

    private:

        MaterialDescriptor m_descriptor;
    };
}

#endif