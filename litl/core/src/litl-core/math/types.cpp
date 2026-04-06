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

    mat3::mat3(mat4 const& other)
    {
        float* selfPtr = dataPtr();
        float const* otherPtr = other.dataPtr();

        selfPtr[0] = otherPtr[0];
        selfPtr[1] = otherPtr[1];
        selfPtr[2] = otherPtr[2];

        selfPtr[3] = otherPtr[4];
        selfPtr[4] = otherPtr[5];
        selfPtr[5] = otherPtr[6];

        selfPtr[6] = otherPtr[8];
        selfPtr[7] = otherPtr[9];
        selfPtr[8] = otherPtr[10];
    }

    mat4::mat4(mat3 const& other)
    {
        float* selfPtr = dataPtr();
        float const* otherPtr = other.dataPtr();

        selfPtr[0]  = otherPtr[0];
        selfPtr[1]  = otherPtr[1];
        selfPtr[2]  = otherPtr[2];
        selfPtr[3]  = 1.0f;

        selfPtr[4]  = otherPtr[3];
        selfPtr[5]  = otherPtr[4];
        selfPtr[6]  = otherPtr[5];
        selfPtr[7]  = 1.0f;

        selfPtr[8]  = otherPtr[6];
        selfPtr[9]  = otherPtr[7];
        selfPtr[10] = otherPtr[8];
        selfPtr[11] = 1.0f;

        selfPtr[12] = 0.0f;
        selfPtr[13] = 0.0f;
        selfPtr[14] = 0.0f;
        selfPtr[15] = 1.0f;
    }
}