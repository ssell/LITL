#include "tests.hpp"
#include "litl-core/math/types.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 default constructor initializes to (0,0,0,1)", "[math::vec4]")
    {
        vec4 v;
        REQUIRE(v.x() == 0.0f);
        REQUIRE(v.y() == 0.0f);
        REQUIRE(v.z() == 0.0f);
        REQUIRE(v.w() == 1.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 component constructor", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(v.x() == 1.0f);
        REQUIRE(v.y() == 2.0f);
        REQUIRE(v.z() == 3.0f);
        REQUIRE(v.w() == 4.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 copy constructor", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 b(a);
        REQUIRE(b == a);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 explicit glm::vec4 constructor", "[math::vec4]")
    {
        glm::vec4 g(5.0f, 6.0f, 7.0f, 8.0f);
        vec4 v(g);
        REQUIRE(v == g);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 equality with vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 c(1.0f, 2.0f, 3.0f, 5.0f);

        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 equality with glm::vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        glm::vec4 g(1.0f, 2.0f, 3.0f, 4.0f);
        glm::vec4 h(9.0f, 2.0f, 3.0f, 4.0f);

        REQUIRE(a == g);
        REQUIRE_FALSE(a == h);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 equality uses epsilon comparison", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        // A value that differs by less than a typical float epsilon should still compare equal
        vec4 b(1.0f + 1e-7f, 2.0f, 3.0f, 4.0f);
        // A value clearly outside epsilon should not
        vec4 c(1.1f, 2.0f, 3.0f, 4.0f);

        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 negation", "[math::vec4]")
    {
        REQUIRE(-vec4(5.0f, -5.0f, 0.0f, 1.0f) == vec4(-5.0, 5.0f, 0.0f, -1.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Addition
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 + scalar", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 r = v + 10.0f;
        REQUIRE(r == vec4(11.0f, 12.0f, 13.0f, 14.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 + vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 b(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((a + b) == vec4(11.0f, 22.0f, 33.0f, 44.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 + glm::vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        glm::vec4 g(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((a + g) == vec4(11.0f, 22.0f, 33.0f, 44.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 += scalar", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        v += 5.0f;
        REQUIRE(v == vec4(6.0f, 7.0f, 8.0f, 9.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 += vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        a += vec4(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE(a == vec4(11.0f, 22.0f, 33.0f, 44.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 += glm::vec4", "[math::vec4]")
    {
        vec4 a(1.0f, 2.0f, 3.0f, 4.0f);
        a += glm::vec4(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE(a == vec4(11.0f, 22.0f, 33.0f, 44.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Subtraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 - scalar", "[math::vec4]")
    {
        vec4 v(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((v - 5.0f) == vec4(5.0f, 15.0f, 25.0f, 35.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 - vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE((a - b) == vec4(9.0f, 18.0f, 27.0f, 36.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 - glm::vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((a - glm::vec4(1.0f, 2.0f, 3.0f, 4.0f)) == vec4(9.0f, 18.0f, 27.0f, 36.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 -= scalar", "[math::vec4]")
    {
        vec4 v(10.0f, 20.0f, 30.0f, 40.0f);
        v -= 5.0f;
        REQUIRE(v == vec4(5.0f, 15.0f, 25.0f, 35.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 -= vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        a -= vec4(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(a == vec4(9.0f, 18.0f, 27.0f, 36.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 -= glm::vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        a -= glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(a == vec4(9.0f, 18.0f, 27.0f, 36.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 * scalar", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE((v * 3.0f) == vec4(3.0f, 6.0f, 9.0f, 12.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 * vec4 (component-wise)", "[math::vec4]")
    {
        vec4 a(2.0f, 3.0f, 4.0f, 5.0f);
        vec4 b(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((a * b) == vec4(20.0f, 60.0f, 120.0f, 200.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 * glm::vec4", "[math::vec4]")
    {
        vec4 a(2.0f, 3.0f, 4.0f, 5.0f);
        REQUIRE((a * glm::vec4(10.0f, 20.0f, 30.0f, 40.0f)) == vec4(20.0f, 60.0f, 120.0f, 200.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 *= scalar", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        v *= 3.0f;
        REQUIRE(v == vec4(3.0f, 6.0f, 9.0f, 12.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 *= vec4", "[math::vec4]")
    {
        vec4 a(2.0f, 3.0f, 4.0f, 5.0f);
        a *= vec4(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE(a == vec4(20.0f, 60.0f, 120.0f, 200.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 *= glm::vec4", "[math::vec4]")
    {
        vec4 a(2.0f, 3.0f, 4.0f, 5.0f);
        a *= glm::vec4(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE(a == vec4(20.0f, 60.0f, 120.0f, 200.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Division
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 / scalar", "[math::vec4]")
    {
        vec4 v(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((v / 2.0f) == vec4(5.0f, 10.0f, 15.0f, 20.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 / vec4 (component-wise)", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        vec4 b(2.0f, 4.0f, 5.0f, 8.0f);
        REQUIRE((a / b) == vec4(5.0f, 5.0f, 6.0f, 5.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 / glm::vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        REQUIRE((a / glm::vec4(2.0f, 4.0f, 5.0f, 8.0f)) == vec4(5.0f, 5.0f, 6.0f, 5.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 /= scalar", "[math::vec4]")
    {
        vec4 v(10.0f, 20.0f, 30.0f, 40.0f);
        v /= 2.0f;
        REQUIRE(v == vec4(5.0f, 10.0f, 15.0f, 20.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 /= vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        a /= vec4(2.0f, 4.0f, 5.0f, 8.0f);
        REQUIRE(a == vec4(5.0f, 5.0f, 6.0f, 5.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 /= glm::vec4", "[math::vec4]")
    {
        vec4 a(10.0f, 20.0f, 30.0f, 40.0f);
        a /= glm::vec4(2.0f, 4.0f, 5.0f, 8.0f);
        REQUIRE(a == vec4(5.0f, 5.0f, 6.0f, 5.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Accessors (mutability)
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 accessors are mutable", "[math::vec4]")
    {
        vec4 v;
        v.x() = 5.0f;
        v.y() = 6.0f;
        v.z() = 7.0f;
        v.w() = 8.0f;
        REQUIRE(v == vec4(5.0f, 6.0f, 7.0f, 8.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 const accessors", "[math::vec4]")
    {
        const vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE(v.x() == 1.0f);
        REQUIRE(v.y() == 2.0f);
        REQUIRE(v.z() == 3.0f);
        REQUIRE(v.w() == 4.0f);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 length", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        // sqrt(1 + 4 + 9 + 16) = sqrt(30) ≈ 5.4772
        REQUIRE(fequals(v.length(), std::sqrt(30.0f)));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 length of unit axis", "[math::vec4]")
    {
        vec4 v(0.0f, 0.0f, 0.0f, 1.0f);
        REQUIRE(isOne(v.length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 normalize mutates in place", "[math::vec4]")
    {
        vec4 v(3.0f, 0.0f, 0.0f, 0.0f);
        v.normalize();
        REQUIRE(v == vec4(1.0f, 0.0f, 0.0f, 0.0f));
        REQUIRE(isOne(v.length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 normalized returns unit vector without mutating", "[math::vec4]")
    {
        vec4 v(0.0f, 4.0f, 0.0f, 0.0f);
        vec4 n = v.normalized();

        REQUIRE(n == vec4(0.0f, 1.0f, 0.0f, 0.0f));
        REQUIRE(isOne(n.length()));
        // Original unchanged
        REQUIRE(v == vec4(0.0f, 4.0f, 0.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 normalized with non-trivial vector", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        vec4 n = v.normalized();
        REQUIRE(isOne(n.length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 setZero() sets all components to zero", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        v.setZero();
        REQUIRE(v == vec4(0.0f, 0.0f, 0.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 isZeroed", "[math::vec4]")
    {
        REQUIRE(vec4(0.0f, 0.0f, 0.0f, 0.0f).isZeroed());
        REQUIRE_FALSE(vec4(0.0f, 0.0f, 0.0f, 1.0f).isZeroed());
        REQUIRE_FALSE(vec4(1.0f, 0.0f, 0.0f, 0.0f).isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 default-constructed is not zeroed (w defaults to 1)", "[math::vec4]")
    {
        vec4 v;
        REQUIRE_FALSE(v.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 isIdentity", "[math::vec4]")
    {
        REQUIRE(vec4(0.0f, 0.0f, 0.0f, 1.0f).isIdentity());
        REQUIRE_FALSE(vec4(0.0f, 0.0f, 0.0f, 0.0f).isIdentity());
        REQUIRE_FALSE(vec4(1.0f, 0.0f, 0.0f, 1.0f).isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 default-constructed is identity", "[math::vec4]")
    {
        vec4 v;
        REQUIRE(v.isIdentity());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Compound / edge-case scenarios
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec4 chained compound assignment", "[math::vec4]")
    {
        vec4 v(1.0f, 1.0f, 1.0f, 1.0f);
        v += vec4(1.0f, 2.0f, 3.0f, 4.0f);
        v *= 2.0f;
        v -= vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // (1+1)*2 - 1 = 3,  (1+2)*2 - 1 = 5,  (1+3)*2 - 1 = 7,  (1+4)*2 - 1 = 9
        REQUIRE(v == vec4(3.0f, 5.0f, 7.0f, 9.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 negative components", "[math::vec4]")
    {
        vec4 a(-1.0f, -2.0f, -3.0f, -4.0f);
        vec4 b(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE((a + b) == vec4(0.0f, 0.0f, 0.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 multiply by zero yields zeroed vector", "[math::vec4]")
    {
        vec4 v(1.0f, 2.0f, 3.0f, 4.0f);
        REQUIRE((v * 0.0f).isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 identity arithmetic (add zero, multiply one)", "[math::vec4]")
    {
        vec4 v(3.0f, 5.0f, 7.0f, 9.0f);
        REQUIRE((v + 0.0f) == v);
        REQUIRE((v * 1.0f) == v);
        REQUIRE((v - 0.0f) == v);
        REQUIRE((v / 1.0f) == v);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec4 setZero() then isZeroed() consistency", "[math::vec4]")
    {
        vec4 v(99.0f, -42.0f, 0.5f, 7.0f);
        v.setZero();
        REQUIRE(v.isZeroed());
        REQUIRE_FALSE(v.isIdentity());
    } LITL_END_TEST_CASE
}