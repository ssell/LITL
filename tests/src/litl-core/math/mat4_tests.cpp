#include "tests.hpp"
#include "litl-core/math/types.hpp"

namespace litl::tests
{
    // -------------------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 default constructor zeroes all elements", "[math::mat4]")
    {
        mat4 m;
        REQUIRE(m.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 copy constructor", "[math::mat4]")
    {
        auto a = mat4::identity();
        mat4 b(a);
        REQUIRE(b.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 construct from glm::mat4", "[math::mat4]")
    {
        glm::mat4 g(1.0f);
        mat4 m(g);
        REQUIRE(m.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 construct from span", "[math::mat4]")
    {
        // Column-major: identity matrix as a flat array
        float vals[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        mat4 m(std::span<float const>(vals, 16));
        REQUIRE(m.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 construct from span shorter than 16 zeroes remainder", "[math::mat4]")
    {
        float vals[4] = { 5.0f, 6.0f, 7.0f, 8.0f };
        mat4 m(std::span<float const>(vals, 4));

        // First four elements (column 0) should match
        REQUIRE(fequals(m.get(0), 5.0f));
        REQUIRE(fequals(m.get(1), 6.0f));
        REQUIRE(fequals(m.get(2), 7.0f));
        REQUIRE(fequals(m.get(3), 8.0f));

        // Remaining should be zero (from default init)
        for (uint32_t i = 4; i < 16; ++i)
        {
            REQUIRE(isZero(m.get(i)));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 from mat3", "[math::mat4]")
    {
        float data[9]{};

        for (auto i = 0; i < 9; ++i)
        {
            data[i] = static_cast<float>(i);
        }

        mat3 m3{ std::span{ data, 9 } };
        mat4 m4{ m3 };

        REQUIRE(fequals(m4.get(0), 0.0f));
        REQUIRE(fequals(m4.get(1), 1.0f));
        REQUIRE(fequals(m4.get(2), 2.0f));
        REQUIRE(fequals(m4.get(3), 1.0f));

        REQUIRE(fequals(m4.get(4), 3.0f));
        REQUIRE(fequals(m4.get(5), 4.0f));
        REQUIRE(fequals(m4.get(6), 5.0f));
        REQUIRE(fequals(m4.get(7), 1.0f));

        REQUIRE(fequals(m4.get(8), 6.0f));
        REQUIRE(fequals(m4.get(9), 7.0f));
        REQUIRE(fequals(m4.get(10), 8.0f));
        REQUIRE(fequals(m4.get(11), 1.0f));

        REQUIRE(fequals(m4.get(12), 0.0f));
        REQUIRE(fequals(m4.get(13), 0.0f));
        REQUIRE(fequals(m4.get(14), 0.0f));
        REQUIRE(fequals(m4.get(15), 1.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Factory methods
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4::identity", "[math::mat4]")
    {
        auto m = mat4::identity();
        REQUIRE(m.isIdentity());
        REQUIRE(fequals(m.determinant(), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::translation", "[math::mat4]")
    {
        auto m = mat4::translation(vec3{ 3.0f, 4.0f, 5.0f });
        REQUIRE(m.isIdentity() == false);

        vec4 t = m.translation();
        REQUIRE(fequals(t.x(), 3.0f));
        REQUIRE(fequals(t.y(), 4.0f));
        REQUIRE(fequals(t.z(), 5.0f));
        REQUIRE(fequals(t.w(), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::scaling", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2.0f, 3.0f, 4.0f });

        REQUIRE(fequals(m.get(0, 0), 2.0f));
        REQUIRE(fequals(m.get(1, 1), 3.0f));
        REQUIRE(fequals(m.get(2, 2), 4.0f));
        REQUIRE(fequals(m.get(3, 3), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::rotation around Z axis by 90 degrees", "[math::mat4]")
    {
        float angle = glm::radians(90.0f);
        auto m = mat4::rotation(angle, vec3{ 0.0f, 0.0f, 1.0f });

        // Rotating (1,0,0) around Z by 90° should give (0,1,0)
        vec3 result = m * vec3{ 1.0f, 0.0f, 0.0f };
        REQUIRE(fequals(result.x(), 0.0f));
        REQUIRE(fequals(result.y(), 1.0f));
        REQUIRE(fequals(result.z(), 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::rotation from euler angles", "[math::mat4]")
    {
        auto m = mat4::rotation(0.0f, 0.0f, 0.0f);
        REQUIRE(m.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::perspective produces non-identity matrix", "[math::mat4]")
    {
        auto m = mat4::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);
        REQUIRE_FALSE(m.isIdentity());
        REQUIRE_FALSE(m.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::orthographic", "[math::mat4]")
    {
        auto m = mat4::orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
        REQUIRE_FALSE(m.isIdentity());
        REQUIRE_FALSE(m.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::lookAt", "[math::mat4]")
    {
        auto m = mat4::lookAt(
            vec3{ 0.0f, 0.0f, 5.0f },
            vec3{ 0.0f, 0.0f, 0.0f },
            vec3{ 0.0f, 1.0f, 0.0f }
        );
        REQUIRE_FALSE(m.isZeroed());
        // Determinant of a pure rotation+translation view matrix should be ±1
        REQUIRE(fequals(std::abs(m.determinant()), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4::fromAxis", "[math::mat4]")
    {
        auto m = mat4::fromAxis(
            vec4{ 1, 0, 0, 0 },
            vec4{ 0, 1, 0, 0 },
            vec4{ 0, 0, 1, 0 },
            vec4{ 0, 0, 0, 1 }
        );
        REQUIRE(m.isIdentity());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
        // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 equality with epsilon tolerance", "[math::mat4]")
    {
        auto a = mat4::identity();
        auto b = mat4::identity();
        REQUIRE(a == b);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 equality with glm::mat4", "[math::mat4]")
    {
        auto a = mat4::identity();
        glm::mat4 g(1.0f);
        REQUIRE(a == g);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 negation", "[math::mat4]")
    {
        auto m = mat4::identity();
        auto neg = -m;

        REQUIRE(fequals(neg.get(0, 0), -1.0f));
        REQUIRE(fequals(neg.get(1, 1), -1.0f));
        REQUIRE(fequals(neg.get(2, 2), -1.0f));
        REQUIRE(fequals(neg.get(3, 3), -1.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Addition
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 + scalar", "[math::mat4]")
    {
        mat4 m;
        auto result = m + 1.0f;
        // All 16 elements should be 1.0
        for (uint32_t i = 0; i < 16; ++i)
        {
            REQUIRE(fequals(result.get(i), 1.0f));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 + mat4", "[math::mat4]")
    {
        auto a = mat4::identity();
        auto b = mat4::identity();
        auto c = a + b;

        REQUIRE(fequals(c.get(0, 0), 2.0f));
        REQUIRE(fequals(c.get(1, 1), 2.0f));
        REQUIRE(fequals(c.get(0, 1), 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 += scalar", "[math::mat4]")
    {
        mat4 m;
        m += 5.0f;
        for (uint32_t i = 0; i < 16; ++i)
        {
            REQUIRE(fequals(m.get(i), 5.0f));
        }
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 += mat4", "[math::mat4]")
    {
        auto a = mat4::identity();
        a += mat4::identity();
        REQUIRE(fequals(a.get(0, 0), 2.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Subtraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 - scalar", "[math::mat4]")
    {
        auto m = mat4::identity();
        auto result = m - 1.0f;

        // Diagonal goes to 0, off-diagonal goes to -1
        REQUIRE(fequals(result.get(0, 0), 0.0f));
        REQUIRE(fequals(result.get(0, 1), -1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 - mat4 yields zero matrix", "[math::mat4]")
    {
        auto a = mat4::identity();
        auto b = mat4::identity();
        auto c = a - b;
        REQUIRE(c.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 -= mat4", "[math::mat4]")
    {
        auto a = mat4::identity();
        a -= mat4::identity();
        REQUIRE(a.isZeroed());
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 * identity = same matrix", "[math::mat4]")
    {
        auto t = mat4::translation(vec3{ 1, 2, 3 });
        auto result = t * mat4::identity();
        REQUIRE(result == t);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 * scalar", "[math::mat4]")
    {
        auto m = mat4::identity();
        auto result = m * 2.0f;
        REQUIRE(fequals(result.get(0, 0), 2.0f));
        REQUIRE(fequals(result.get(1, 1), 2.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 *= scalar", "[math::mat4]")
    {
        auto m = mat4::identity();
        m *= 3.0f;
        REQUIRE(fequals(m.get(0, 0), 3.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 * vec3 applies translation", "[math::mat4]")
    {
        auto t = mat4::translation(vec3{ 10.0f, 20.0f, 30.0f });
        vec3 v{ 1.0f, 2.0f, 3.0f };
        vec3 result = t * v;

        REQUIRE(fequals(result.x(), 11.0f));
        REQUIRE(fequals(result.y(), 22.0f));
        REQUIRE(fequals(result.z(), 33.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 * vec4", "[math::mat4]")
    {
        auto s = mat4::scaling(vec3{ 2.0f, 2.0f, 2.0f });
        vec4 v{ 3.0f, 4.0f, 5.0f, 1.0f };
        vec4 result = s * v;

        REQUIRE(fequals(result.x(), 6.0f));
        REQUIRE(fequals(result.y(), 8.0f));
        REQUIRE(fequals(result.z(), 10.0f));
        REQUIRE(fequals(result.w(), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 combined TRS multiplication order", "[math::mat4]")
    {
        // model = T * R * S  applies S first, then R, then T
        auto T = mat4::translation(vec3{ 0.0f, 0.0f, 5.0f });
        auto S = mat4::scaling(vec3{ 2.0f, 2.0f, 2.0f });
        auto model = T * mat4::identity() * S; // skip rotation for simplicity

        vec3 origin{ 0.0f, 0.0f, 0.0f };
        vec3 result = model * origin;

        // Scale does nothing to origin, translation moves to (0,0,5)
        REQUIRE(fequals(result.x(), 0.0f));
        REQUIRE(fequals(result.y(), 0.0f));
        REQUIRE(fequals(result.z(), 5.0f));

        // Non-origin point: scale then translate
        vec3 p{ 1.0f, 0.0f, 0.0f };
        vec3 r2 = model * p;
        REQUIRE(fequals(r2.x(), 2.0f));
        REQUIRE(fequals(r2.z(), 5.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Division
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 / scalar", "[math::mat4]")
    {
        auto m = mat4::identity() * 4.0f;
        auto result = m / 2.0f;
        REQUIRE(fequals(result.get(0, 0), 2.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 /= scalar", "[math::mat4]")
    {
        auto m = mat4::identity() * 6.0f;
        m /= 3.0f;
        REQUIRE(fequals(m.get(0, 0), 2.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Getters / Setters
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 set/get by col,row", "[math::mat4]")
    {
        mat4 m;
        m.set(2, 1, 42.0f);
        REQUIRE(fequals(m.get(2, 1), 42.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 set/get by flat index", "[math::mat4]")
    {
        mat4 m;
        m.set(7, 99.0f); // col 1, row 3
        REQUIRE(fequals(m.get(7), 99.0f));
        REQUIRE(fequals(m.get(1, 3), 99.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 setCol / col", "[math::mat4]")
    {
        mat4 m;
        m.setCol(1, 10.0f, 20.0f, 30.0f, 40.0f);

        vec4 c = m.col(1);
        REQUIRE(fequals(c.x(), 10.0f));
        REQUIRE(fequals(c.y(), 20.0f));
        REQUIRE(fequals(c.z(), 30.0f));
        REQUIRE(fequals(c.w(), 40.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 setRow / row", "[math::mat4]")
    {
        mat4 m;
        m.setRow(2, 5.0f, 6.0f, 7.0f, 8.0f);

        vec4 r = m.row(2);
        REQUIRE(fequals(r.x(), 5.0f));
        REQUIRE(fequals(r.y(), 6.0f));
        REQUIRE(fequals(r.z(), 7.0f));
        REQUIRE(fequals(r.w(), 8.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 operator[] column access", "[math::mat4]")
    {
        auto m = mat4::identity();
        // operator[] returns float* to start of column
        REQUIRE(fequals(m[0][0], 1.0f));
        REQUIRE(fequals(m[0][1], 0.0f));
        REQUIRE(fequals(m[1][1], 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 translation() getter extracts column 3", "[math::mat4]")
    {
        auto m = mat4::translation(vec3{ 7, 8, 9 });
        vec4 t = m.translation();
        REQUIRE(fequals(t.x(), 7.0f));
        REQUIRE(fequals(t.y(), 8.0f));
        REQUIRE(fequals(t.z(), 9.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 scale() extracts column lengths", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2.0f, 3.0f, 4.0f });
        vec3 s = m.scale();
        REQUIRE(fequals(s.x(), 2.0f));
        REQUIRE(fequals(s.y(), 3.0f));
        REQUIRE(fequals(s.z(), 4.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 right/up/forward on identity", "[math::mat4]")
    {
        auto m = mat4::identity();
        vec3 r = m.right();
        vec3 u = m.up();
        vec3 f = m.forward();

        REQUIRE(fequals(r.x(), 1.0f));
        REQUIRE(fequals(u.y(), 1.0f));
        REQUIRE(fequals(f.z(), 1.0f));
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 zero() and isZeroed()", "[math::mat4]")
    {
        auto m = mat4::identity();
        REQUIRE_FALSE(m.isZeroed());
        m.zero();
        REQUIRE(m.isZeroed());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 setIdentity() and isIdentity()", "[math::mat4]")
    {
        mat4 m;
        REQUIRE_FALSE(m.isIdentity());
        m.setIdentity();
        REQUIRE(m.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 transpose", "[math::mat4]")
    {
        mat4 m;
        m.set(0, 1, 5.0f); // col 0, row 1
        auto t = m.transposed();
        REQUIRE(fequals(t.get(1, 0), 5.0f)); // should be at col 1, row 0

        // In-place version
        m.transpose();
        REQUIRE(m == t);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 transpose of identity is identity", "[math::mat4]")
    {
        REQUIRE(mat4::identity().transposed().isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 inverse of identity is identity", "[math::mat4]")
    {
        auto inv = mat4::identity().inverted();
        REQUIRE(inv.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 M * M^-1 = identity", "[math::mat4]")
    {
        auto m = mat4::translation(vec3{ 3, 4, 5 });
        auto result = m * m.inverted();
        REQUIRE(result.isIdentity());
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 inverse in-place", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2, 2, 2 });
        auto expected = m.inverted();
        m.inverse();
        REQUIRE(m == expected);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 inverseTranspose", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2, 3, 4 });
        auto it = m.inverseTranspose();
        // Should equal transpose(inverse(m))
        auto manual = m.inverted().transposed();
        REQUIRE(it == manual);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 determinant of identity is 1", "[math::mat4]")
    {
        REQUIRE(fequals(mat4::identity().determinant(), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 determinant of scaling matrix", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2.0f, 3.0f, 4.0f });
        REQUIRE(fequals(m.determinant(), 24.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 transform(vec3) same as operator*", "[math::mat4]")
    {
        auto m = mat4::translation(vec3{ 1, 2, 3 });
        vec3 v{ 10, 20, 30 };

        vec3 a = m.transform(v);
        vec3 b = m * v;

        REQUIRE(fequals(a.x(), b.x()));
        REQUIRE(fequals(a.y(), b.y()));
        REQUIRE(fequals(a.z(), b.z()));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 transformDirection ignores translation", "[math::mat4]")
    {
        auto m = mat4::translation(vec3{ 100, 200, 300 });
        vec3 dir{ 1, 0, 0 };
        vec3 result = m.transformDirection(dir);

        // Direction should be unchanged by pure translation
        REQUIRE(fequals(result.x(), 1.0f));
        REQUIRE(fequals(result.y(), 0.0f));
        REQUIRE(fequals(result.z(), 0.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 in-place translate", "[math::mat4]")
    {
        auto m = mat4::identity();
        m.translate(vec3{ 5, 6, 7 });

        vec4 t = m.translation();
        REQUIRE(fequals(t.x(), 5.0f));
        REQUIRE(fequals(t.y(), 6.0f));
        REQUIRE(fequals(t.z(), 7.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 in-place rotate", "[math::mat4]")
    {
        auto m = mat4::identity();
        m.rotate(glm::radians(90.0f), vec3{ 0, 0, 1 });

        vec3 result = m * vec3{ 1, 0, 0 };
        REQUIRE(fequals(result.x(), 0.0f));
        REQUIRE(fequals(result.y(), 1.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 in-place scale", "[math::mat4]")
    {
        auto m = mat4::identity();
        m.scale(vec3{ 3, 3, 3 });

        vec3 s = m.scale();
        REQUIRE(fequals(s.x(), 3.0f));
        REQUIRE(fequals(s.y(), 3.0f));
        REQUIRE(fequals(s.z(), 3.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 get3x3 extracts upper-left 3x3", "[math::mat4]")
    {
        auto m = mat4::scaling(vec3{ 2, 3, 4 });
        mat3 m3 = m.get3x3();

        // The 3x3 diagonal should match
        REQUIRE(fequals(m3.get(0, 0), 2.0f));
        REQUIRE(fequals(m3.get(1, 1), 3.0f));
        REQUIRE(fequals(m3.get(2, 2), 4.0f));
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 toString produces non-empty string", "[math::mat4]")
    {
        auto m = mat4::identity();
        std::string s = m.toString();
        REQUIRE_FALSE(s.empty());
        REQUIRE(s.find("C0:") != std::string::npos);
        REQUIRE(s.find("C3:") != std::string::npos);
    } LITL_END_TEST_CASE

    // -------------------------------------------------------------------------------------
   // Data access
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat4 dataPtr gives contiguous column-major access", "[math::mat4]")
    {
        auto m = mat4::identity();
        float const* p = m.dataPtr();

        // col 0, row 0 = 1
        REQUIRE(p[0] == 1.0f);
        // col 0, row 1 = 0
        REQUIRE(p[1] == 0.0f);
        // col 1, row 1 = 1  (index 5)
        REQUIRE(p[5] == 1.0f);
    } LITL_END_TEST_CASE

    LITL_TEST_CASE("mat4 data() returns reference to underlying glm::mat4", "[math::mat4]")
    {
        auto m = mat4::identity();
        glm::mat4& ref = m.data();
        ref[0][0] = 42.0f;
        REQUIRE(m.get(0, 0) == 42.0f);
    } LITL_END_TEST_CASE
}