#include "tests.hpp"
#include "litl-core/math/types.hpp"

namespace litl::tests
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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 component constructor", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        REQUIRE(v.x() == 1.0f);
        REQUIRE(v.y() == 2.0f);
        REQUIRE(v.z() == 3.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 copy constructor", "[math::vec3]")
    {
        vec3 a(4.0f, 5.0f, 6.0f);
        vec3 b(a);
        REQUIRE(b == a);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 explicit construction from glm::vec3", "[math::vec3]")
    {
        glm::vec3 g(7.0f, 8.0f, 9.0f);
        vec3 v(g);
        REQUIRE(v == g);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 equality uses approximate comparison", "[math::vec3]")
    {
        vec3 a(1.0f, 2.0f, 3.0f);
        vec3 b(1.0f, 2.0f, 3.0f);
        REQUIRE(a == b);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 equality with glm::vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        glm::vec3 g(1.0f, 2.0f, 3.0f);
        REQUIRE(v == g);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 inequality for differing components", "[math::vec3]")
    {
        vec3 a(1.0f, 2.0f, 3.0f);
        vec3 b(1.0f, 2.0f, 4.0f);
        REQUIRE_FALSE(a == b);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 equality tolerates floating-point drift", "[math::vec3]")
    {
        // Accumulate small additions that would fail exact ==
        vec3 v(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < 10; ++i)
            v += vec3(0.1f, 0.1f, 0.1f);

        REQUIRE(v == vec3(1.0f, 1.0f, 1.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 negation", "[math::vec3]")
    {
        REQUIRE(-vec3(5.0f, -5.0f, 0.0f) == vec3(-5.0, 5.0f, 0.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Addition
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 + scalar", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) + 1.0f == vec3(2.0f, 3.0f, 4.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 + vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) + vec3(4.0f, 5.0f, 6.0f) == vec3(5.0f, 7.0f, 9.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 + glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 0.0f, 0.0f) + glm::vec3(0.0f, 1.0f, 0.0f) == vec3(1.0f, 1.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 += scalar", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v += 10.0f;
        REQUIRE(v == vec3(11.0f, 12.0f, 13.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 += vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v += vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(v == vec3(2.0f, 3.0f, 4.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 += glm::vec3", "[math::vec3]")
    {
        vec3 v(0.0f, 0.0f, 0.0f);
        v += glm::vec3(5.0f, 6.0f, 7.0f);
        REQUIRE(v == vec3(5.0f, 6.0f, 7.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Subtraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 - scalar", "[math::vec3]")
    {
        REQUIRE(vec3(5.0f, 5.0f, 5.0f) - 2.0f == vec3(3.0f, 3.0f, 3.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 - vec3", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) - vec3(1.0f, 2.0f, 3.0f) == vec3(9.0f, 18.0f, 27.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 - glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 1.0f, 1.0f) - glm::vec3(1.0f, 1.0f, 1.0f) == vec3(0.0f, 0.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 -= scalar", "[math::vec3]")
    {
        vec3 v(10.0f, 10.0f, 10.0f);
        v -= 3.0f;
        REQUIRE(v == vec3(7.0f, 7.0f, 7.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 -= vec3", "[math::vec3]")
    {
        vec3 v(5.0f, 5.0f, 5.0f);
        v -= vec3(5.0f, 5.0f, 5.0f);
        REQUIRE(v.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 -= glm::vec3", "[math::vec3]")
    {
        vec3 v(3.0f, 4.0f, 5.0f);
        v -= glm::vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(v == vec3(2.0f, 3.0f, 4.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 * scalar", "[math::vec3]")
    {
        REQUIRE(vec3(1.0f, 2.0f, 3.0f) * 2.0f == vec3(2.0f, 4.0f, 6.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 * vec3 (component-wise)", "[math::vec3]")
    {
        REQUIRE(vec3(2.0f, 3.0f, 4.0f) * vec3(5.0f, 6.0f, 7.0f) == vec3(10.0f, 18.0f, 28.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 * glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(2.0f, 2.0f, 2.0f) * glm::vec3(3.0f, 3.0f, 3.0f) == vec3(6.0f, 6.0f, 6.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 *= scalar", "[math::vec3]")
    {
        vec3 v(1.0f, 2.0f, 3.0f);
        v *= 0.5f;
        REQUIRE(v == vec3(0.5f, 1.0f, 1.5f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 *= vec3", "[math::vec3]")
    {
        vec3 v(2.0f, 3.0f, 4.0f);
        v *= vec3(2.0f, 2.0f, 2.0f);
        REQUIRE(v == vec3(4.0f, 6.0f, 8.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 *= glm::vec3", "[math::vec3]")
    {
        vec3 v(1.0f, 1.0f, 1.0f);
        v *= glm::vec3(10.0f, 20.0f, 30.0f);
        REQUIRE(v == vec3(10.0f, 20.0f, 30.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 * 0 yields zero", "[math::vec3]")
    {
        vec3 v = vec3(42.0f, 99.0f, -7.0f) * 0.0f;
        REQUIRE(v.isZeroed());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Division
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 / scalar", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) / 10.0f == vec3(1.0f, 2.0f, 3.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 / vec3 (component-wise)", "[math::vec3]")
    {
        REQUIRE(vec3(10.0f, 20.0f, 30.0f) / vec3(2.0f, 4.0f, 5.0f) == vec3(5.0f, 5.0f, 6.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 / glm::vec3", "[math::vec3]")
    {
        REQUIRE(vec3(9.0f, 9.0f, 9.0f) / glm::vec3(3.0f, 3.0f, 3.0f) == vec3(3.0f, 3.0f, 3.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 /= scalar", "[math::vec3]")
    {
        vec3 v(4.0f, 8.0f, 16.0f);
        v /= 4.0f;
        REQUIRE(v == vec3(1.0f, 2.0f, 4.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 /= vec3", "[math::vec3]")
    {
        vec3 v(100.0f, 200.0f, 300.0f);
        v /= vec3(10.0f, 20.0f, 30.0f);
        REQUIRE(v == vec3(10.0f, 10.0f, 10.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 /= glm::vec3", "[math::vec3]")
    {
        vec3 v(6.0f, 8.0f, 10.0f);
        v /= glm::vec3(2.0f, 4.0f, 5.0f);
        REQUIRE(v == vec3(3.0f, 2.0f, 2.0f));
    } LITL_END_TEST_CASE

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
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 const accessors return correct values", "[math::vec3]")
    {
        const vec3 v(7.0f, 8.0f, 9.0f);
        REQUIRE(v.x() == 7.0f);
        REQUIRE(v.y() == 8.0f);
        REQUIRE(v.z() == 9.0f);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Factory
    // -------------------------------------------------------------------------------------
    
    LITL_TEST_CASE("handiness", "[math::vec3]")
    {
        // LITL is left-handed (right == +x, up == +y, forward == +z)
        vec3 right = vec3::right();
        vec3 up = vec3::up();
        vec3 forward = vec3::forward();
        vec3 calcForward = right.cross(up);

        REQUIRE(right == vec3{1.0f, 0.0f, 0.0f});
        REQUIRE(up == vec3{ 0.0f, 1.0f, 0.0f });
        REQUIRE(forward == vec3{ 0.0f, 0.0f, 1.0f });
        REQUIRE(calcForward == forward);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 length of unit axes", "[math::vec3]")
    {
        REQUIRE(isOne(vec3(1.0f, 0.0f, 0.0f).length()));
        REQUIRE(isOne(vec3(0.0f, 1.0f, 0.0f).length()));
        REQUIRE(isOne(vec3(0.0f, 0.0f, 1.0f).length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 length of (3,4,0) is 5", "[math::vec3]")
    {
        REQUIRE(fequals(vec3(3.0f, 4.0f, 0.0f).length(), 5.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 length of zero vector is 0", "[math::vec3]")
    {
        REQUIRE(isZero(vec3().length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 normalize mutates in-place", "[math::vec3]")
    {
        vec3 v(3.0f, 0.0f, 0.0f);
        v.normalize();
        REQUIRE(v == vec3(1.0f, 0.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 normalized returns unit-length copy", "[math::vec3]")
    {
        vec3 v(0.0f, 5.0f, 0.0f);
        vec3 n = v.normalized();

        // original unchanged
        REQUIRE(v == vec3(0.0f, 5.0f, 0.0f));
        // result is unit length
        REQUIRE(isOne(n.length()));
        REQUIRE(n == vec3(0.0f, 1.0f, 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 normalized of arbitrary vector has length 1", "[math::vec3]")
    {
        vec3 v(3.0f, 4.0f, 5.0f);
        REQUIRE(isOne(v.normalized().length()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 setZero() clears all components", "[math::vec3]")
    {
        vec3 v(99.0f, -42.0f, 7.0f);
        v.setZero();
        REQUIRE(v.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 isZeroed on default-constructed vec3", "[math::vec3]")
    {
        REQUIRE(vec3().isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 isZeroed is false for non-zero vector", "[math::vec3]")
    {
        REQUIRE_FALSE(vec3(0.0f, 0.0f, 0.001f).isZeroed());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Compound / edge-case scenarios
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 chained arithmetic", "[math::vec3]")
    {
        // (1,2,3) * 2 + (1,1,1) = (3,5,7)
        vec3 result = vec3(1.0f, 2.0f, 3.0f) * 2.0f + vec3(1.0f, 1.0f, 1.0f);
        REQUIRE(result == vec3(3.0f, 5.0f, 7.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 negative components", "[math::vec3]")
    {
        vec3 a(-1.0f, -2.0f, -3.0f);
        vec3 b(1.0f, 2.0f, 3.0f);
        REQUIRE((a + b).isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 compound assignment returns *this", "[math::vec3]")
    {
        vec3 v(1.0f, 1.0f, 1.0f);
        // chain two compound assignments
        (v += vec3(1.0f, 1.0f, 1.0f)) *= 3.0f;
        REQUIRE(v == vec3(6.0f, 6.0f, 6.0f));
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // Min and Max
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("vec3 min", "[math::vec3]")
    {
        vec3 a{ 5.0f, 10.0f, 15.0f };
        vec3 b{ -5.0f, 100.0f, 0.0f };
        vec3 expected{ -5.0f, 10.0f, 0.0f };

        REQUIRE(min(a, b) == expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("vec3 max", "[math::vec3]")
    {
        vec3 a{ 5.0f, 10.0f, 15.0f };
        vec3 b{ -5.0f, 100.0f, 0.0f };
        vec3 expected{ 5.0f, 100.0f, 15.0f };

        REQUIRE(max(a, b) == expected);
    } LITL_END_TEST_CASE
        
    // -------------------------------------------------------------------------------------
    // distanceTo
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("distanceTo", "[math::vec3]")
    {
        vec3 a{ 0.0f, 0.0f, 0.0f };
        vec3 b{ 10.0f, 0.0f, 0.0f };
        vec3 c{ 100.0f, 100.0f, 100.0f };
        vec3 d{ -100.0f, -10.0f, 10.0f };

        constexpr float expectedAtoB = 10.0f;
        constexpr float expectedAtoC = 173.205080757f;
        constexpr float expectedAtoD = 100.995049384f;
        constexpr float expectedBtoC = 167.630546142f;
        constexpr float expectedBtoD = 110.905365064f;
        constexpr float expectedCtoD = 245.356882928f;

        REQUIRE(isZero(a.distanceTo(a)));
        REQUIRE(isZero(b.distanceTo(b)));
        REQUIRE(isZero(c.distanceTo(c)));
        REQUIRE(isZero(d.distanceTo(d)));

        REQUIRE(fequals(a.distanceTo(b), expectedAtoB));
        REQUIRE(fequals(b.distanceTo(a), expectedAtoB));

        REQUIRE(fequals(a.distanceTo(c), expectedAtoC));
        REQUIRE(fequals(c.distanceTo(a), expectedAtoC));

        REQUIRE(fequals(a.distanceTo(d), expectedAtoD));
        REQUIRE(fequals(d.distanceTo(a), expectedAtoD));

        REQUIRE(fequals(b.distanceTo(c), expectedBtoC));
        REQUIRE(fequals(c.distanceTo(b), expectedBtoC));

        REQUIRE(fequals(b.distanceTo(d), expectedBtoD));
        REQUIRE(fequals(d.distanceTo(b), expectedBtoD));

        REQUIRE(fequals(c.distanceTo(d), expectedCtoD));
        REQUIRE(fequals(d.distanceTo(c), expectedCtoD));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("distanceSqTo", "[math::vec3]")
    {
        vec3 a{ 0.0f, 0.0f, 0.0f };
        vec3 b{ 10.0f, 0.0f, 0.0f };
        vec3 c{ 100.0f, 100.0f, 100.0f };
        vec3 d{ -100.0f, -10.0f, 10.0f };

        constexpr float expectedAtoB = 100.0f;
        constexpr float expectedAtoC = 30000.0f;
        constexpr float expectedAtoD = 10200.0f;
        constexpr float expectedBtoC = 28100.0f;
        constexpr float expectedBtoD = 12300.0f;
        constexpr float expectedCtoD = 60200.0f;

        REQUIRE(isZero(a.distanceSqTo(a)));
        REQUIRE(isZero(b.distanceSqTo(b)));
        REQUIRE(isZero(c.distanceSqTo(c)));
        REQUIRE(isZero(d.distanceSqTo(d)));

        REQUIRE(fequals(a.distanceSqTo(b), expectedAtoB));
        REQUIRE(fequals(b.distanceSqTo(a), expectedAtoB));

        REQUIRE(fequals(a.distanceSqTo(c), expectedAtoC));
        REQUIRE(fequals(c.distanceSqTo(a), expectedAtoC));

        REQUIRE(fequals(a.distanceSqTo(d), expectedAtoD));
        REQUIRE(fequals(d.distanceSqTo(a), expectedAtoD));

        REQUIRE(fequals(b.distanceSqTo(c), expectedBtoC));
        REQUIRE(fequals(c.distanceSqTo(b), expectedBtoC));

        REQUIRE(fequals(b.distanceSqTo(d), expectedBtoD));
        REQUIRE(fequals(d.distanceSqTo(b), expectedBtoD));

        REQUIRE(fequals(c.distanceSqTo(d), expectedCtoD));
        REQUIRE(fequals(d.distanceSqTo(c), expectedCtoD));
    } LITL_END_TEST_CASE
}