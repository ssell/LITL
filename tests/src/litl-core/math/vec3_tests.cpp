#include "tests.hpp"
#include "litl-core/math/vec3.hpp"
#include "litl-core/math/math.hpp"

namespace LITL::Math::Tests
{
    LITL_TEST_CASE("Basic", "[math::vec3]")
    {
        Vec3 v{};
        glm::vec3 glmvec{10.0f, 100.0f, 10.0f};

        REQUIRE(isZero(v.x()));
        REQUIRE(isZero(v.y()));
        REQUIRE(isZero(v.z()));

        v.x() = 10.0f;
        v.y() = 100.0f;
        v.z() = 10.0f;

        REQUIRE(fequals(v.x(), 10.0f));
        REQUIRE(fequals(v.y(), 100.0f));
        REQUIRE(fequals(v.z(), 10.0f));

        Vec3 normalized = v.normalized();
        glmvec = glm::normalize(glmvec);

        REQUIRE(normalized == glmvec);

        Vec3 scaled = normalized * 100.0f;
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

    } END_LITL_TEST_CASE
}