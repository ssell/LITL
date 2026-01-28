#ifndef LITL_MATH_VERTEX_H__
#define LITL_MATH_VERTEX_H__

#include <glm/glm.hpp>

namespace LITL::Math
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };
}
#endif