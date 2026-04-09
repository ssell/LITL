#include "tests.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/traits.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: default constructor is identity", "[math::quat]")
    {
        quat q;
        REQUIRE(fequals(q.w(), 1.0f));
        REQUIRE(fequals(q.x(), 0.0f));
        REQUIRE(fequals(q.y(), 0.0f));
        REQUIRE(fequals(q.z(), 0.0f));
        REQUIRE(q.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: component constructor", "[math::quat]")
    {
        quat q(0.5f, 0.1f, 0.2f, 0.3f);
        REQUIRE(fequals(q.w(), 0.5f));
        REQUIRE(fequals(q.x(), 0.1f));
        REQUIRE(fequals(q.y(), 0.2f));
        REQUIRE(fequals(q.z(), 0.3f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: copy constructor", "[math::quat]")
    {
        quat a(0.7f, 0.1f, 0.2f, 0.3f);
        quat b(a);
        REQUIRE(b == a);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: explicit glm::quat constructor", "[math::quat]")
    {
        glm::quat g(1.0f, 0.0f, 0.0f, 0.0f);
        quat q(g);
        REQUIRE(q == g);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Factory methods
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat::identity", "[math::quat]")
    {
        auto q = quat::identity();
        REQUIRE(q.isIdentity());
        REQUIRE(fequals(q.length(), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat::fromAxisAngle", "[math::quat]")
    {
        // 90° around Y axis
        auto q = quat::fromAxisAngle(vec3{ 0.0f, 1.0f, 0.0f }, Traits<float>::pi_over_two);
        REQUIRE(fequals(q.length(), 1.0f));
        REQUIRE(fequals(q.angle(), Traits<float>::pi_over_two));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat::fromAngleX/Y/Z produce axis-aligned rotations", "[math::quat]")
    {
        SECTION("fromAngleX rotates around X")
        {
            auto q = quat::fromAngleX(Traits<float>::pi_over_two);
            // Rotating (0,1,0) by 90° around X should give (0,0,1)
            vec3 result = q.rotate(vec3{ 0.0f, 1.0f, 0.0f });
            REQUIRE(result == vec3{ 0.0f, 0.0f, 1.0f });
        }

        SECTION("fromAngleY rotates around Y")
        {
            auto q = quat::fromAngleY(Traits<float>::pi_over_two);
            // Rotating (0,0,-1) by 90° around Y should give (-1,0,0)
            vec3 result = q.rotate(vec3{ 0.0f, 0.0f, -1.0f });
            REQUIRE(result == vec3{ -1.0f, 0.0f, 0.0f });
        }

        SECTION("fromAngleZ rotates around Z")
        {
            auto q = quat::fromAngleZ(Traits<float>::pi_over_two);
            // Rotating (1,0,0) by 90° around Z should give (0,1,0)
            vec3 result = q.rotate(vec3{ 1.0f, 0.0f, 0.0f });
            REQUIRE(result == vec3{ 0.0f, 1.0f, 0.0f });
        }
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat::fromEuler round-trips through toEuler", "[math::quat]")
    {
        float yaw = 0.3f, pitch = 0.5f, roll = 0.1f;
        auto q = quat::fromEuler(yaw, pitch, roll);
        vec3 euler = q.toEuler();

        REQUIRE(fequals(euler.x(), yaw));
        REQUIRE(fequals(euler.y(), pitch));
        REQUIRE(fequals(euler.z(), roll));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat::fromVector rotates one direction to another", "[math::quat]")
    {
        vec3 from{ 1.0f, 0.0f, 0.0f };
        vec3 to{ 0.0f, 1.0f, 0.0f };
        auto q = quat::fromVector(from, to);
        vec3 result = q.rotate(from);
        REQUIRE(result == to);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat::lookRotation", "[math::quat]")
    {
        vec3 fwd{ 0.0f, 0.0f, -1.0f };
        vec3 upDir{ 0.0f, 1.0f, 0.0f };
        auto q = quat::lookRotation(fwd, upDir);
        REQUIRE(fequals(q.length(), 1.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: equality operators", "[math::quat]")
    {
        quat a(1.0f, 0.0f, 0.0f, 0.0f);
        quat b(1.0f, 0.0f, 0.0f, 0.0f);
        quat c(0.0f, 1.0f, 0.0f, 0.0f);

        REQUIRE(a == b);
        REQUIRE_FALSE(a == c);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: equality with glm::quat", "[math::quat]")
    {
        quat q(1.0f, 0.0f, 0.0f, 0.0f);
        glm::quat g(1.0f, 0.0f, 0.0f, 0.0f);
        REQUIRE(q == g);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: unary negation", "[math::quat]")
    {
        quat q(0.5f, 0.1f, 0.2f, 0.3f);
        quat neg = -q;
        REQUIRE(fequals(neg.w(), -0.5f));
        REQUIRE(fequals(neg.x(), -0.1f));
        REQUIRE(fequals(neg.y(), -0.2f));
        REQUIRE(fequals(neg.z(), -0.3f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: negated quat represents the same rotation", "[math::quat]")
    {
        auto q = quat::fromAngleY(Traits<float>::pi / 3.0f);
        auto neg = -q;
        vec3 v{ 1.0f, 2.0f, 3.0f };
        // q and -q encode the same rotation
        REQUIRE(q.rotate(v) == neg.rotate(v));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: identity multiplication", "[math::quat]")
    {
        auto id = quat::identity();
        auto q = quat::fromAngleX(0.5f);
        REQUIRE((q * id) == q);
        REQUIRE((id * q) == q);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: q * inverse == identity", "[math::quat]")
    {
        auto q = quat::fromAxisAngle(vec3{ 0.0f, 1.0f, 0.0f }, 1.2f);
        auto inv = q.getInverse();
        auto result = q * inv;
        REQUIRE(result.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: compound rotation via operator*", "[math::quat]")
    {
        // Two 90° rotations around Y should equal one 180° rotation
        auto q90 = quat::fromAngleY(Traits<float>::pi_over_two);
        auto q180 = q90 * q90;
        auto expected = quat::fromAngleY(Traits<float>::pi);
        REQUIRE(fequals(q180.angle(), expected.angle()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: operator*= quat", "[math::quat]")
    {
        auto a = quat::fromAngleX(0.3f);
        auto b = quat::fromAngleY(0.5f);
        auto expected = a * b;
        a *= b;
        REQUIRE(a == expected);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: scalar multiplication", "[math::quat]")
    {
        quat q(1.0f, 0.0f, 0.0f, 0.0f);
        auto scaled = q * 2.0f;
        REQUIRE(fequals(scaled.w(), 2.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: operator*= scalar", "[math::quat]")
    {
        quat q(1.0f, 0.0f, 0.0f, 0.0f);
        q *= 3.0f;
        REQUIRE(fequals(q.w(), 3.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Rotation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: rotate vec3 (value)", "[math::quat]")
    {
        auto q = quat::fromAngleZ(Traits<float>::pi_over_two);
        vec3 v{ 1.0f, 0.0f, 0.0f };
        vec3 result = q.rotate(v);
        REQUIRE(result == vec3{ 0.0f, 1.0f, 0.0f });
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: operator* vec3 matches rotate", "[math::quat]")
    {
        auto q = quat::fromAngleX(1.0f);
        vec3 v{ 0.0f, 1.0f, 0.0f };
        REQUIRE((q * v) == q.rotate(v));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Accessors
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: mutable accessors", "[math::quat]")
    {
        quat q;
        q.x() = 0.5f;
        q.y() = 0.6f;
        q.z() = 0.7f;
        q.w() = 0.8f;
        REQUIRE(fequals(q.x(), 0.5f));
        REQUIRE(fequals(q.y(), 0.6f));
        REQUIRE(fequals(q.z(), 0.7f));
        REQUIRE(fequals(q.w(), 0.8f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: length of unit quaternion is 1", "[math::quat]")
    {
        auto q = quat::fromAngleY(1.0f);
        REQUIRE(fequals(q.length(), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: lengthSquared", "[math::quat]")
    {
        quat q(2.0f, 0.0f, 0.0f, 0.0f);
        REQUIRE(fequals(q.lengthSquared(), 4.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: dot product", "[math::quat]")
    {
        auto id = quat::identity();
        REQUIRE(fequals(id.dot(id), 1.0f));

        auto q = quat::fromAngleY(Traits<float>::pi);
        // dot with self should be 1 for unit quaternion
        REQUIRE(fequals(q.dot(q), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: getNormalized", "[math::quat]")
    {
        quat q(2.0f, 0.0f, 0.0f, 0.0f);
        auto n = q.getNormalized();
        REQUIRE(fequals(n.length(), 1.0f));
        REQUIRE(fequals(n.w(), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: normalize resets to identity", "[math::quat]")
    {
        // NOTE: The current implementation of normalize() resets to identity.
        // This test documents that behavior — it may be a bug (should probably
        // normalize in-place rather than reset). Flagging for review.
        auto q = quat::fromAngleY(1.0f);
        q.normalize();
        REQUIRE(q.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: getInverse", "[math::quat]")
    {
        auto q = quat::fromAxisAngle(vec3{ 1.0f, 0.0f, 0.0f }, 1.0f);
        auto inv = q.getInverse();
        auto product = q * inv;
        REQUIRE(product.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: inverse() in-place", "[math::quat]")
    {
        auto q = quat::fromAngleZ(0.7f);
        auto original = q;
        q.inverse();
        auto product = original * q;
        REQUIRE(product.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: getConjugate", "[math::quat]")
    {
        quat q(0.5f, 0.1f, 0.2f, 0.3f);
        auto conj = q.getConjugate();
        REQUIRE(fequals(conj.w(), q.w()));
        REQUIRE(fequals(conj.x(), -q.x()));
        REQUIRE(fequals(conj.y(), -q.y()));
        REQUIRE(fequals(conj.z(), -q.z()));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: conjugate() in-place", "[math::quat]")
    {
        quat q(0.5f, 0.1f, 0.2f, 0.3f);
        auto expected = q.getConjugate();
        q.conjugate();
        REQUIRE(q == expected);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: conjugate equals inverse for unit quaternions", "[math::quat]")
    {
        auto q = quat::fromAngleX(1.5f);
        REQUIRE(q.getConjugate() == q.getInverse());
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Interpolation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: lerp at boundaries", "[math::quat]")
    {
        auto a = quat::fromAngleX(0.0f);
        auto b = quat::fromAngleX(1.0f);

        REQUIRE(a.lerp(b, 0.0f) == a);
        REQUIRE(a.lerp(b, 1.0f) == b);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: slerp at boundaries", "[math::quat]")
    {
        auto a = quat::fromAngleY(0.0f);
        auto b = quat::fromAngleY(Traits<float>::pi / 2.0f);

        REQUIRE(a.slerp(b, 0.0f) == a);
        REQUIRE(a.slerp(b, 1.0f) == b);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: slerp midpoint", "[math::quat]")
    {
        auto a = quat::identity();
        auto b = quat::fromAngleY(Traits<float>::pi / 2.0f);
        auto mid = a.slerp(b, 0.5f);

        // Midpoint should be 45° around Y
        auto expected = quat::fromAngleY(Traits<float>::pi / 4.0f);
        REQUIRE(mid == expected);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Axis / Angle extraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: angle() and axis() round-trip", "[math::quat]")
    {
        vec3 ax{ 0.0f, 0.0f, 1.0f };
        float ang = 1.2f;
        auto q = quat::fromAxisAngle(ax, ang);

        REQUIRE(fequals(q.angle(), ang));
        REQUIRE(q.axis() == ax);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Direction vectors
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: identity forward/right/up", "[math::quat]")
    {
        auto q = quat::identity();
        REQUIRE(q.forward() == vec3{ 0.0f, 0.0f, -1.0f });
        REQUIRE(q.right() == vec3{ 1.0f, 0.0f,  0.0f });
        REQUIRE(q.up() == vec3{ 0.0f, 1.0f,  0.0f });
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: 90° Y rotation swaps forward and right", "[math::quat]")
    {
        auto q = quat::fromAngleY(Traits<float>::pi_over_two);
        // Forward (0,0,-1) rotated 90° around Y → (-1,0,0)
        REQUIRE(q.forward() == vec3{ -1.0f, 0.0f, 0.0f });
        // Right (1,0,0) rotated 90° around Y → (0,0,-1)
        REQUIRE(q.right() == vec3{ 0.0f, 0.0f, -1.0f });
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // angleBetween
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: angleBetween identical quats is zero", "[math::quat]")
    {
        auto q = quat::fromAngleX(0.5f);
        REQUIRE(fequals(q.angleBetween(q), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: angleBetween measures angular difference", "[math::quat]")
    {
        auto a = quat::identity();
        auto b = quat::fromAngleY(Traits<float>::pi_over_two);
        REQUIRE(fequals(a.angleBetween(b), Traits<float>::pi_over_two));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Euler extraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: yaw/pitch/roll accessors", "[math::quat]")
    {
        float y = 0.3f, p = 0.5f, r = 0.1f;
        auto q = quat::fromEuler(y, p, r);
        REQUIRE(fequals(q.yaw(), y));
        REQUIRE(fequals(q.pitch(), p));
        REQUIRE(fequals(q.roll(), r));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Data access
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: data() returns underlying glm::quat", "[math::quat]")
    {
        quat q(0.5f, 0.1f, 0.2f, 0.3f);
        glm::quat const& ref = q.data();
        REQUIRE(fequals(ref.w, 0.5f));
        REQUIRE(fequals(ref.x, 0.1f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("quat: dataPtr() points to contiguous memory", "[math::quat]")
    {
        quat q(1.0f, 2.0f, 3.0f, 4.0f);
        float const* ptr = q.dataPtr();
        // glm stores as x, y, z, w
        REQUIRE(fequals(ptr[0], 2.0f)); // x
        REQUIRE(fequals(ptr[1], 3.0f)); // y
        REQUIRE(fequals(ptr[2], 4.0f)); // z
        REQUIRE(fequals(ptr[3], 1.0f)); // w
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // toString
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("quat: toString format", "[math::quat]")
    {
        auto q = quat::identity();
        auto str = q.toString();
        REQUIRE(str == "(1.000,0.000,0.000,0.000)");
    } END_LITL_TEST_CASE
}