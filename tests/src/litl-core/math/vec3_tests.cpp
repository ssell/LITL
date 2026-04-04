#include "tests.hpp"
#include "litl-core/math/vec3.hpp"

namespace LITL::Math::Tests
{
    // -------------------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 default constructs to zero", "[math::vec3]")
    {
        vec3 v;
        REQUIRE(v.x() == 0.0f);
        REQUIRE(v.y() == 0.0f);
        REQUIRE(v.z() == 0.0f);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 component constructor", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        REQUIRE(v.x() == 1.0f);
        REQUIRE(v.y() == 2.0f);
        REQUIRE(v.z() == 3.0f);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 copy constructor", "[math::vec3]")
    {
        vec3 a(4.0f, 5.0f, 6.0f);
        vec3 b(a);
        REQUIRE(b == a);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 explicit construction from glm::vec3", "[math::vec3]")
    {
        glm::vec3 g(7.0f, 8.0f, 9.0f);
        vec3 v(g);
        REQUIRE(v == g);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 equality uses approximate comparison", "[math::vec3]")
    {
        vec3 a(1.0f, 2.0f, 3.0f);
        vec3 b(1.0f, 2.0f, 3.0f);
        REQUIRE(a == b);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 equality with glm::vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        glm::vec3 g(1.0f, 2.0f, 3.0f);
        REQUIRE(v == g);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 inequality for differing components", "[math::vec3]")
    {
        vec3 a(1.0f, 2.0f, 3.0f);
        vec3 b(1.0f, 2.0f, 4.0f);
        REQUIRE_FALSE(a == b);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 equality tolerates floating-point drift", "[math::vec3]")
    {
        // Accumulate small additions that would fail exact ==
        vec3 v(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < 10; ++i)
            v += vec3(0.1f, 0.1f, 0.1f);

        REQUIRE(v == vec3(1.0f, 1.0f, 1.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 negation", "[math::vec3]")
    {
        REQUIRE(-vec3(5.0f, -5.0f, 0.0f) == vec3(-5.0, 5.0f, 0.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Addition
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 + scalar", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) + 1.0f == vec3(2.0f, 3.0f, 4.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 + vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) + vec3(4.0f, 5.0f, 6.0f) == vec3(5.0f, 7.0f, 9.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 + glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 0.0f, 0.0f) + glm::vec3(0.0f, 1.0f, 0.0f) == vec3(1.0f, 1.0f, 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 += scalar", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v += 10.0f;
        REQUIRE(v == vec3(11.0f, 12.0f, 13.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 += vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v += vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(v == vec3(2.0f, 3.0f, 4.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 += glm::vec3", "[math::vec3]")
    {
        vec3 v(0.0f, 0.0f, 0.0f);
        v += glm::vec3(5.0f, 6.0f, 7.0f);
        REQUIRE(v == vec3(5.0f, 6.0f, 7.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Subtraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 - scalar", "[math::vec3]")
    {
        REQUIRE(vec3(5.0f, 5.0f, 5.0f) - 2.0f == vec3(3.0f, 3.0f, 3.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 - vec3", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) - vec3(1.0f, 2.0f, 3.0f) == vec3(9.0f, 18.0f, 27.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 - glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 1.0f, 1.0f) - glm::vec3(1.0f, 1.0f, 1.0f) == vec3(0.0f, 0.0f, 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 -= scalar", "[math::vec3]")
    {
        vec3 v(10.0f, 10.0f, 10.0f);
        v -= 3.0f;
        REQUIRE(v == vec3(7.0f, 7.0f, 7.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 -= vec3", "[math::vec3]")
    {
        vec3 v(5.0f, 5.0f, 5.0f);
        v -= vec3(5.0f, 5.0f, 5.0f);
        REQUIRE(v.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 -= glm::vec3", "[math::vec3]")
    {
        vec3 v(3.0f, 4.0f, 5.0f);
        v -= glm::vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(v == vec3(2.0f, 3.0f, 4.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 * scalar", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) * 2.0f == vec3(2.0f, 4.0f, 6.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 * vec3 (component-wise)", "[math::vec3]")
    {
        REQUIRE(vec3(2.0f, 3.0f, 4.0f) * vec3(5.0f, 6.0f, 7.0f) == vec3(10.0f, 18.0f, 28.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 * glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(2.0f, 2.0f, 2.0f) * glm::vec3(3.0f, 3.0f, 3.0f) == vec3(6.0f, 6.0f, 6.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 *= scalar", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v *= 0.5f;
        REQUIRE(v == vec3(0.5f, 1.0f, 1.5f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 *= vec3", "[math::vec3]")
    {
        vec3 v(2.0f, 3.0f, 4.0f);
        v *= vec3(2.0f, 2.0f, 2.0f);
        REQUIRE(v == vec3(4.0f, 6.0f, 8.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 *= glm::vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 1.0f, 1.0f);
        v *= glm::vec3(10.0f, 20.0f, 30.0f);
        REQUIRE(v == vec3(10.0f, 20.0f, 30.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 * 0 yields zero", "[math::vec3]")
    {
        vec3 v = vec3(42.0f, 99.0f, -7.0f) * 0.0f;
        REQUIRE(v.isZeroed());
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Division
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 / scalar", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) / 10.0f == vec3(1.0f, 2.0f, 3.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 / vec3 (component-wise)", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) / vec3(2.0f, 4.0f, 5.0f) == vec3(5.0f, 5.0f, 6.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 / glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(9.0f, 9.0f, 9.0f) / glm::vec3(3.0f, 3.0f, 3.0f) == vec3(3.0f, 3.0f, 3.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 /= scalar", "[math::vec3]")
    {
        vec3 v(4.0f, 8.0f, 16.0f);
        v /= 4.0f;
        REQUIRE(v == vec3(1.0f, 2.0f, 4.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 /= vec3", "[math::vec3]")
    {
        vec3 v(100.0f, 200.0f, 300.0f);
        v /= vec3(10.0f, 20.0f, 30.0f);
        REQUIRE(v == vec3(10.0f, 10.0f, 10.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 /= glm::vec3", "[math::vec3]")
    {
        vec3 v(6.0f, 8.0f, 10.0f);
        v /= glm::vec3(2.0f, 4.0f, 5.0f);
        REQUIRE(v == vec3(3.0f, 2.0f, 2.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Accessors (mutability)
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 mutable accessors modify components", "[math::vec3]")
    {
        vec3 v;
        v.x() = 1.0f;
        v.y() = 2.0f;
        v.z() = 3.0f;
        REQUIRE(v == vec3(1.0f, 2.0f, 3.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 const accessors return correct values", "[math::vec3]")
    {
        const vec3 v(7.0f, 8.0f, 9.0f);
        REQUIRE(v.x() == 7.0f);
        REQUIRE(v.y() == 8.0f);
        REQUIRE(v.z() == 9.0f);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 length of unit axes", "[math::vec3]")
    {
        REQUIRE(isOne(vec3(1.0f, 0.0f, 0.0f).length()));
        REQUIRE(isOne(vec3(0.0f, 1.0f, 0.0f).length()));
        REQUIRE(isOne(vec3(0.0f, 0.0f, 1.0f).length()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 length of (3,4,0) is 5", "[math::vec3]")
    {
        REQUIRE(fequals(vec3(3.0f, 4.0f, 0.0f).length(), 5.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 length of zero vector is 0", "[math::vec3]")
    {
        REQUIRE(isZero(vec3().length()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 normalize mutates in-place", "[math::vec3]")
    {
        vec3 v(3.0f, 0.0f, 0.0f);
        v.normalize();
        REQUIRE(v == vec3(1.0f, 0.0f, 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 normalized returns unit-length copy", "[math::vec3]")
    {
        vec3 v(0.0f, 5.0f, 0.0f);
        vec3 n = v.normalized();

        // original unchanged
        REQUIRE(v == vec3(0.0f, 5.0f, 0.0f));
        // result is unit length
        REQUIRE(isOne(n.length()));
        REQUIRE(n == vec3(0.0f, 1.0f, 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 normalized of arbitrary vector has length 1", "[math::vec3]")
    {
        vec3 v(3.0f, 4.0f, 5.0f);
        REQUIRE(isOne(v.normalized().length()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 zero() clears all components", "[math::vec3]")
    {
        vec3 v(99.0f, -42.0f, 7.0f);
        v.zero();
        REQUIRE(v.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 isZeroed on default-constructed vec3", "[math::vec3]")
    {
        REQUIRE(vec3().isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 isZeroed is false for non-zero vector", "[math::vec3]")
    {
        REQUIRE_FALSE(vec3(0.0f, 0.0f, 0.001f).isZeroed());
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Compound / edge-case scenarios
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 chained arithmetic", "[math::vec3]")
    {
        // (1,2,3) * 2 + (1,1,1) = (3,5,7)
        vec3 result = vec3(1.0f, 2.0f, 3.0f) * 2.0f + vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(result == vec3(3.0f, 5.0f, 7.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 negative components", "[math::vec3]")
    {
        vec3 a(-1.0f, -2.0f, -3.0f);
        vec3 b(1.0f, 2.0f, 3.0f);
        REQUIRE((a + b).isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("vec3 compound assignment returns *this", "[math::vec3]")
    {
        vec3 v(1.0f, 1.0f, 1.0f);
        // chain two compound assignments
        (v += vec3(1.0f, 1.0f, 1.0f)) *= 3.0f;
        REQUIRE(v == vec3(6.0f, 6.0f, 6.0f));
    } END_LITL_TEST_CASE
}