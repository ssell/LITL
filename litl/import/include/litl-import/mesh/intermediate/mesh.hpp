#ifndef LITL_IMPORT_MESH_H__
#define LITL_IMPORT_MESH_H__

#include <vector>

#include "litl-core/math.hpp"
#include "litl-import/mesh/intermediate/primitive.hpp"

namespace litl::import
{
    /// <summary>
    /// Composed of one or more primitives.
    /// </summary>
    struct Mesh
    {
        std::vector<Primitive> primitives;
        PrimitiveType primitiveType{ PrimitiveType::Unknown };
    };
}

#endif