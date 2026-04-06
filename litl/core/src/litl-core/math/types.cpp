#include "litl-core/math/types.hpp"

namespace LITL
{
    vec3::vec3(vec4 const& other)
        : value{ other.x(), other.y(), other.z() }
    {

    }

    vec4::vec4(vec3 const& other)
        : value{ other.x(), other.y(), other.z(), 0.0f }
    {

    }
}