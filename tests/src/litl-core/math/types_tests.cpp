#include "tests.hpp"
#include "litl-core/math.hpp"

namespace litl::tests
{
    LITL_TEST_CASE("mat4 -> quat -> mat3 roundtrip", "[math::types]")
    {
        mat4 mat4LookAt = mat4::lookAt(vec3(0.0f, 0.5f, 0.0f), vec3(0.0f, 0.5f, 0.5f), vec3::up());
        quat quatLookAt = quat{ mat4LookAt };
        mat3 mat3LookAt = mat3{ quatLookAt };

        mat3 expected = mat4LookAt.get3x3();

        REQUIRE(mat3LookAt == expected);
    } END_LITL_TEST_CASE
}