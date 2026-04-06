#include <array>
#include "tests.hpp"
#include "litl-core/math/types.hpp"
#include "litl-core/math/traits.hpp"

namespace LITL::Math::Tests
{
    // -------------------------------------------------------------------------------------
    // Construction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 default constructs to zero matrix", "[mat3][construction]")
    {
        mat3 m;
        REQUIRE(m.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 copy construction", "[mat3][construction]")
    {
        auto m = mat3::identity();
        mat3 copy{ m };
        REQUIRE(copy == m);
        REQUIRE(copy.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 construct from glm::mat3", "[mat3][construction]")
    {
        glm::mat3 g{ 1.0f };
        mat3 m{ g };
        REQUIRE(m.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 construct from span", "[mat3][construction]")
    {
        // Column-major: col0 = {1,2,3}, col1 = {4,5,6}, col2 = {7,8,9}
        std::array<float, 9> values{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        mat3 m{ std::span<float const>{ values } };

        REQUIRE(Math::fequals(m.get(0, 0), 1.0f));
        REQUIRE(Math::fequals(m.get(0, 1), 2.0f));
        REQUIRE(Math::fequals(m.get(0, 2), 3.0f));
        REQUIRE(Math::fequals(m.get(1, 0), 4.0f));
        REQUIRE(Math::fequals(m.get(1, 1), 5.0f));
        REQUIRE(Math::fequals(m.get(1, 2), 6.0f));
        REQUIRE(Math::fequals(m.get(2, 0), 7.0f));
        REQUIRE(Math::fequals(m.get(2, 1), 8.0f));
        REQUIRE(Math::fequals(m.get(2, 2), 9.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 construct from partial span pads with zero", "[mat3][construction]")
    {
        std::array<float, 4> values{ 1, 2, 3, 4 };
        mat3 m{ std::span<float const>{ values } };

        REQUIRE(Math::fequals(m.get(0), 1.0f));
        REQUIRE(Math::fequals(m.get(1), 2.0f));
        REQUIRE(Math::fequals(m.get(2), 3.0f));
        REQUIRE(Math::fequals(m.get(3), 4.0f));
        // Remaining elements stay at default (zero)
        REQUIRE(Math::fequals(m.get(4), 0.0f));
        REQUIRE(Math::fequals(m.get(8), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 from mat4", "[math::mat4]")
    {
        float data[16]{};

        for (auto i = 0; i < 16; ++i)
        {
            data[i] = static_cast<float>(i);
        }

        mat4 m4{ std::span{ data, 16 } };
        mat3 m3{ m4 };

        REQUIRE(Math::fequals(m3.get(0), 0.0f));
        REQUIRE(Math::fequals(m3.get(1), 1.0f));
        REQUIRE(Math::fequals(m3.get(2), 2.0f));

        REQUIRE(Math::fequals(m3.get(3), 4.0f));
        REQUIRE(Math::fequals(m3.get(4), 5.0f));
        REQUIRE(Math::fequals(m3.get(5), 6.0f));

        REQUIRE(Math::fequals(m3.get(6), 8.0f));
        REQUIRE(Math::fequals(m3.get(7), 9.0f));
        REQUIRE(Math::fequals(m3.get(8), 10.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Factory methods
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3::identity", "[mat3][factory]")
    {
        auto m = mat3::identity();
        REQUIRE(m.isIdentity());
        REQUIRE(Math::fequals(m.get(0, 0), 1.0f));
        REQUIRE(Math::fequals(m.get(1, 1), 1.0f));
        REQUIRE(Math::fequals(m.get(2, 2), 1.0f));
        REQUIRE(Math::fequals(m.get(0, 1), 0.0f));
        REQUIRE(Math::fequals(m.get(1, 0), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3::rotation around Z axis by 90 degrees", "[mat3][factory]")
    {
        constexpr float angle = Math::Traits<float>::pi_over_two;
        auto m = mat3::rotation(angle, vec3{ 0, 0, 1 });

        // Rotating (1,0,0) by 90° around Z should give (0,1,0)
        auto result = m * vec3{ 1, 0, 0 };
        REQUIRE(Math::fequals(result.x(), 0.0f));
        REQUIRE(Math::fequals(result.y(), 1.0f));
        REQUIRE(Math::fequals(result.z(), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3::rotation with Euler angles", "[mat3][factory]")
    {
        // Rotating only around Z by 90°
        constexpr float angle = Math::Traits<float>::pi_over_two;
        auto m = mat3::rotation(0.0f, 0.0f, angle);

        auto result = m * vec3{ 1, 0, 0 };
        REQUIRE(Math::fequals(result.x(), 0.0f));
        REQUIRE(Math::fequals(result.y(), 1.0f));
        REQUIRE(Math::fequals(result.z(), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3::scaling", "[mat3][factory]")
    {
        auto m = mat3::scaling(vec3{ 2, 3, 4 });

        REQUIRE(Math::fequals(m.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(m.get(1, 1), 3.0f));
        REQUIRE(Math::fequals(m.get(2, 2), 4.0f));

        auto result = m * vec3{ 1, 1, 1 };
        REQUIRE(Math::fequals(result.x(), 2.0f));
        REQUIRE(Math::fequals(result.y(), 3.0f));
        REQUIRE(Math::fequals(result.z(), 4.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3::fromAxes", "[mat3][factory]")
    {
        vec3 right{ 1, 0, 0 };
        vec3 up{ 0, 1, 0 };
        vec3 forward{ 0, 0, 1 };

        auto m = mat3::fromAxes(right, up, forward);
        REQUIRE(m.isIdentity());
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Equality
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 equality uses epsilon comparison", "[mat3][equality]")
    {
        auto a = mat3::identity();
        auto b = mat3::identity();
        REQUIRE(a == b);

        // Comparing with glm::mat3
        glm::mat3 g{ 1.0f };
        REQUIRE(a == g);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 inequality", "[mat3][equality]")
    {
        auto a = mat3::identity();
        mat3 b;
        REQUIRE_FALSE(a == b);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Negation
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 negation", "[mat3][negation]")
    {
        std::array<float, 9> values{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        mat3 m{ std::span<float const>{ values } };
        auto neg = -m;

        for (uint32_t i = 0; i < 9; ++i)
        {
            REQUIRE(Math::fequals(neg.get(i), -m.get(i)));
        }
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Addition
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 + scalar", "[mat3][addition]")
    {
        auto m = mat3::identity();
        auto result = m + 1.0f;

        // Diagonal becomes 2.0, off-diagonal becomes 1.0
        REQUIRE(Math::fequals(result.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(result.get(0, 1), 1.0f));
        REQUIRE(Math::fequals(result.get(1, 1), 2.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 + mat3", "[mat3][addition]")
    {
        auto a = mat3::identity();
        auto b = mat3::identity();
        auto result = a + b;

        REQUIRE(Math::fequals(result.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(result.get(1, 1), 2.0f));
        REQUIRE(Math::fequals(result.get(2, 2), 2.0f));
        REQUIRE(Math::fequals(result.get(0, 1), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 += scalar", "[mat3][addition]")
    {
        auto m = mat3::identity();
        m += 1.0f;
        REQUIRE(Math::fequals(m.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(m.get(0, 1), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 += mat3", "[mat3][addition]")
    {
        auto m = mat3::identity();
        m += mat3::identity();
        REQUIRE(Math::fequals(m.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(m.get(0, 1), 0.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Subtraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 - scalar", "[mat3][subtraction]")
    {
        auto m = mat3::identity();
        auto result = m - 1.0f;

        REQUIRE(Math::fequals(result.get(0, 0), 0.0f));
        REQUIRE(Math::fequals(result.get(0, 1), -1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 - mat3", "[mat3][subtraction]")
    {
        auto a = mat3::identity();
        auto b = mat3::identity();
        auto result = a - b;
        REQUIRE(result.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 -= scalar", "[mat3][subtraction]")
    {
        auto m = mat3::identity();
        m -= 1.0f;
        REQUIRE(Math::fequals(m.get(0, 0), 0.0f));
        REQUIRE(Math::fequals(m.get(0, 1), -1.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Multiplication
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 * scalar", "[mat3][multiplication]")
    {
        auto m = mat3::identity();
        auto result = m * 3.0f;
        REQUIRE(Math::fequals(result.get(0, 0), 3.0f));
        REQUIRE(Math::fequals(result.get(1, 1), 3.0f));
        REQUIRE(Math::fequals(result.get(0, 1), 0.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 * mat3 (identity)", "[mat3][multiplication]")
    {
        auto a = mat3::identity();
        std::array<float, 9> vals{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        mat3 b{ std::span<float const>{ vals } };

        auto result = a * b;
        REQUIRE(result == b);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 * vec3", "[mat3][multiplication]")
    {
        auto m = mat3::identity();
        vec3 v{ 1, 2, 3 };
        auto result = m * v;
        REQUIRE(result == v);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 *= scalar", "[mat3][multiplication]")
    {
        auto m = mat3::identity();
        m *= 5.0f;
        REQUIRE(Math::fequals(m.get(0, 0), 5.0f));
        REQUIRE(Math::fequals(m.get(1, 1), 5.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 *= mat3", "[mat3][multiplication]")
    {
        auto m = mat3::identity();
        std::array<float, 9> vals{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        mat3 b{ std::span<float const>{ vals } };

        m *= b;
        REQUIRE(m == b);
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Division
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 / scalar", "[mat3][division]")
    {
        auto m = mat3::identity() * 4.0f;
        auto result = m / 2.0f;
        REQUIRE(Math::fequals(result.get(0, 0), 2.0f));
        REQUIRE(Math::fequals(result.get(1, 1), 2.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 /= scalar", "[mat3][division]")
    {
        auto m = mat3::identity() * 6.0f;
        m /= 3.0f;
        REQUIRE(Math::fequals(m.get(0, 0), 2.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Setters / Getters
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 set/get by col,row", "[mat3][accessors]")
    {
        mat3 m;
        m.set(1, 2, 42.0f);
        REQUIRE(Math::fequals(m.get(1, 2), 42.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 set/get by flat index", "[mat3][accessors]")
    {
        mat3 m;
        // Index 4 = col1, row1 (column-major: col*3 + row)
        m.set(4, 99.0f);
        REQUIRE(Math::fequals(m.get(4), 99.0f));
        REQUIRE(Math::fequals(m.get(1, 1), 99.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 operator[] column access", "[mat3][accessors]")
    {
        auto m = mat3::identity();
        REQUIRE(Math::fequals(m[0][0], 1.0f));
        REQUIRE(Math::fequals(m[0][1], 0.0f));
        REQUIRE(Math::fequals(m[1][1], 1.0f));

        // Mutable access
        m[2][0] = 7.0f;
        REQUIRE(Math::fequals(m.get(2, 0), 7.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 setCol / col", "[mat3][accessors]")
    {
        mat3 m;
        m.setCol(1, 10.0f, 20.0f, 30.0f);

        auto c = m.col(1);
        REQUIRE(Math::fequals(c.x(), 10.0f));
        REQUIRE(Math::fequals(c.y(), 20.0f));
        REQUIRE(Math::fequals(c.z(), 30.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 setCol with vec3", "[mat3][accessors]")
    {
        mat3 m;
        m.setCol(0, vec3{ 5, 6, 7 });

        auto c = m.col(0);
        REQUIRE(Math::fequals(c.x(), 5.0f));
        REQUIRE(Math::fequals(c.y(), 6.0f));
        REQUIRE(Math::fequals(c.z(), 7.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 setRow / row", "[mat3][accessors]")
    {
        mat3 m;
        m.setRow(0, 1.0f, 2.0f, 3.0f);

        auto r = m.row(0);
        REQUIRE(Math::fequals(r.x(), 1.0f));
        REQUIRE(Math::fequals(r.y(), 2.0f));
        REQUIRE(Math::fequals(r.z(), 3.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 setRow with vec3", "[mat3][accessors]")
    {
        mat3 m;
        m.setRow(2, vec3{ 11, 12, 13 });

        auto r = m.row(2);
        REQUIRE(Math::fequals(r.x(), 11.0f));
        REQUIRE(Math::fequals(r.y(), 12.0f));
        REQUIRE(Math::fequals(r.z(), 13.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Basis extraction
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 right/up/forward on identity", "[mat3][basis]")
    {
        auto m = mat3::identity();

        REQUIRE(m.right() == vec3{ 1, 0, 0 });
        REQUIRE(m.up() == vec3{ 0, 1, 0 });
        REQUIRE(m.forward() == vec3{ 0, 0, 1 });
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 scale extraction", "[mat3][basis]")
    {
        auto m = mat3::scaling(vec3{ 2, 3, 4 });
        auto s = m.scale();
        REQUIRE(Math::fequals(s.x(), 2.0f));
        REQUIRE(Math::fequals(s.y(), 3.0f));
        REQUIRE(Math::fequals(s.z(), 4.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Utility
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 zero()", "[mat3][utility]")
    {
        auto m = mat3::identity();
        m.zero();
        REQUIRE(m.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 setIdentity()", "[mat3][utility]")
    {
        mat3 m;
        m.setIdentity();
        REQUIRE(m.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 transpose", "[mat3][utility]")
    {
        mat3 m;
        m.setCol(0, 1, 2, 3);
        m.setCol(1, 4, 5, 6);
        m.setCol(2, 7, 8, 9);

        auto t = m.transposed();

        // Transposed: rows become columns
        REQUIRE(Math::fequals(t.get(0, 0), 1.0f));
        REQUIRE(Math::fequals(t.get(0, 1), 4.0f));
        REQUIRE(Math::fequals(t.get(0, 2), 7.0f));
        REQUIRE(Math::fequals(t.get(1, 0), 2.0f));
        REQUIRE(Math::fequals(t.get(1, 1), 5.0f));
        REQUIRE(Math::fequals(t.get(1, 2), 8.0f));

        // In-place version
        mat3 m2 = m;
        m2.transpose();
        REQUIRE(m2 == t);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 transpose is self-inverse", "[mat3][utility]")
    {
        mat3 m;
        m.setCol(0, 1, 2, 3);
        m.setCol(1, 4, 5, 6);
        m.setCol(2, 7, 8, 9);

        REQUIRE(m.transposed().transposed() == m);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 determinant of identity is 1", "[mat3][utility]")
    {
        REQUIRE(Math::fequals(mat3::identity().determinant(), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 determinant of scaled matrix", "[mat3][utility]")
    {
        auto m = mat3::scaling(vec3{ 2, 3, 4 });
        REQUIRE(Math::fequals(m.determinant(), 24.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 inverse", "[mat3][utility]")
    {
        auto m = mat3::scaling(vec3{ 2, 4, 8 });
        auto inv = m.inverted();

        REQUIRE(Math::fequals(inv.get(0, 0), 0.5f));
        REQUIRE(Math::fequals(inv.get(1, 1), 0.25f));
        REQUIRE(Math::fequals(inv.get(2, 2), 0.125f));

        // M * M^-1 == I
        auto product = m * inv;
        REQUIRE(product.isIdentity());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 in-place inverse", "[mat3][utility]")
    {
        auto m = mat3::scaling(vec3{ 2, 4, 8 });
        auto expected = m.inverted();
        m.inverse();
        REQUIRE(m == expected);
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 inverseTranspose", "[mat3][utility]")
    {
        auto m = mat3::scaling(vec3{ 2, 3, 4 });
        auto it = m.inverseTranspose();

        // For a diagonal matrix, inverse-transpose is just inverse
        REQUIRE(Math::fequals(it.get(0, 0), 0.5f));
        REQUIRE(Math::fequals(it.get(1, 1), 1.0f / 3.0f));
        REQUIRE(Math::fequals(it.get(2, 2), 0.25f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 transform is equivalent to operator*", "[mat3][utility]")
    {
        auto m = mat3::scaling(vec3{ 2, 3, 4 });
        vec3 v{ 1, 1, 1 };

        REQUIRE(m.transform(v) == (m * v));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 rotate in-place", "[mat3][utility]")
    {
        auto m = mat3::identity();
        constexpr float angle = Math::Traits<float>::pi_over_two;
        m.rotate(angle, vec3{ 0, 0, 1 });

        auto result = m * vec3{ 1, 0, 0 };
        REQUIRE(Math::fequals(result.x(), 0.0f));
        REQUIRE(Math::fequals(result.y(), 1.0f));
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 scale in-place", "[mat3][utility]")
    {
        auto m = mat3::identity();
        m.scale(vec3{ 2, 3, 4 });

        auto s = m.scale();
        REQUIRE(Math::fequals(s.x(), 2.0f));
        REQUIRE(Math::fequals(s.y(), 3.0f));
        REQUIRE(Math::fequals(s.z(), 4.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // Data access
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 data() returns reference to glm::mat3", "[mat3][access]")
    {
        auto m = mat3::identity();
        glm::mat3& ref = m.data();
        ref = glm::mat3{ 0.0f };
        REQUIRE(m.isZeroed());
    } END_LITL_TEST_CASE

    LITL_TEST_CASE("mat3 dataPtr() provides contiguous float access", "[mat3][access]")
    {
        auto m = mat3::identity();
        float const* ptr = m.dataPtr();

        // Column-major layout: [1, 0, 0,  0, 1, 0,  0, 0, 1]
        REQUIRE(Math::fequals(ptr[0], 1.0f));
        REQUIRE(Math::fequals(ptr[1], 0.0f));
        REQUIRE(Math::fequals(ptr[4], 1.0f));
        REQUIRE(Math::fequals(ptr[8], 1.0f));
    } END_LITL_TEST_CASE

    // -------------------------------------------------------------------------------------
    // toString
    // -------------------------------------------------------------------------------------

    LITL_TEST_CASE("mat3 toString produces expected format", "[mat3][string]")
    {
        auto m = mat3::identity();
        auto s = m.toString();

        REQUIRE(s.find("C0:") != std::string::npos);
        REQUIRE(s.find("C1:") != std::string::npos);
        REQUIRE(s.find("C2:") != std::string::npos);
    } END_LITL_TEST_CASE
}