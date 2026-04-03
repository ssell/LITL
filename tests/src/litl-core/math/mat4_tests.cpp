#include "tests.hpp"
#include "litl-core/math/mat4.hpp"

namespace LITL::Math::Tests
{
    LITL_TEST_CASE("Basic", "[math::mat4]")
    {
        Mat4 matrix{ 1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f };

    } END_LITL_TEST_CASE
}