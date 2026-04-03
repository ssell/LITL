#include "tests.hpp"
#include "litl-core/math/vec4.hpp"

namespace LITL::Math::Tests
{
    LITL_TEST_CASE("Basic", "[math::vec4]")
    {
        Vec4 v{};
        glm::vec4 glmvec{ 10.0f, 100.0f, 10.0f, 1000.0f };

        REQUIRE(isZero(v.x()));
        REQUIRE(isZero(v.y()));
        REQUIRE(isZero(v.z()));

        v.x() = 10.0f;
        v.y() = 100.0f;
        v.z() = 10.0f;
        v.w() = 1000.0f;

        REQUIRE(fequals(v.x(), 10.0f));
        REQUIRE(fequals(v.y(), 100.0f));
        REQUIRE(fequals(v.z(), 10.0f));
        REQUIRE(fequals(v.w(), 1000.0f));

        Vec4 normalized = v.normalized();
        glmvec = glm::normalize(glmvec);

        REQUIRE(normalized == glmvec);

        Vec4 scaled = normalized * 100.0f;
        glmvec = glmvec * 100.0f;

        REQUIRE(scaled == glmvec);

        scaled = scaled / 10.0f;
        glmvec = glmvec / 10.0f;

        REQUIRE(scaled == glmvec);

        scaled *= 100.0f;
        glmvec *= 100.0f;

        REQUIRE(scaled == glmvec);

        scaled /= 10.0f;
        glmvec /= 10.0f;

        REQUIRE(scaled == glmvec);
        REQUIRE(fequals(scaled.length(), glm::length(glmvec)));

        REQUIRE(v.isZero() == false);
        v.zero();
        REQUIRE(v.isZero() == true);

    } END_LITL_TEST_CASE
}