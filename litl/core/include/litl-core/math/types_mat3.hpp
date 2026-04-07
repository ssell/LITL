#ifndef LITL_MATH_MAT3_H__
#define LITL_MATH_MAT3_H__

// for euler angle support
#define GLM_ENABLE_EXPERIMENTAL

#include <cassert>
#include <format>
#include <span>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "litl-core/math/math.hpp"
#include "litl-core/types.hpp"
#include "litl-core/math/types_vec3.hpp"

namespace LITL
{
    struct mat4;
    struct quat;

    /// <summary>
    /// Column-major: [col][row], (mat * vec), etc.
    /// Memory is stored as: [col0.x, col0.y, col0.z, col0.w, col1.x, ...]
    /// </summary>
    struct mat3
    {

        constexpr mat3() {}
        constexpr mat3(mat3 const& other) : value(other.value) {}
        constexpr explicit mat3(glm::mat3 const& other) : value(other) {}
        explicit mat3(mat4 const& other);
        explicit mat3(quat const& quaternion);

        constexpr explicit mat3(std::span<float const> values)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(values.size()) && (i < 9); ++i)
            {
                set(i, values[i]);
            }
        }

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(mat3 const& other) const noexcept
        {
            return glm::all(glm::equal(value, other.value, Math::Traits<float>::epsilon));
        }

        [[nodiscard]] constexpr bool operator==(glm::mat3 const& other) const noexcept
        {
            return glm::all(glm::equal(value, other, Math::Traits<float>::epsilon));
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat3 operator-() const noexcept
        {
            return mat3{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Addition
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat3 operator+(float scalar) const noexcept
        {
            return mat3{ value + scalar };
        }

        [[nodiscard]] constexpr mat3 operator+(mat3 const& other) const noexcept
        {
            return mat3{ value + other.value };
        }

        [[nodiscard]] constexpr mat3 operator+(glm::mat3 const& other) const noexcept
        {
            return mat3{ value + other };
        }

        [[nodiscard]] constexpr mat3 operator+(vec3 const& other) const noexcept
        {
            glm::mat3 m = value;
            m += other.data();
            return mat3{ m };
        }

        constexpr mat3& operator+=(float scalar) noexcept
        {
            value += scalar;
            return *this;
        }

        constexpr mat3& operator+=(mat3 const& other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr mat3& operator+=(glm::vec3 const& other) noexcept
        {
            value += other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Subtraction
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat3 operator-(float scalar) const noexcept
        {
            return mat3{ value - scalar };
        }

        [[nodiscard]] constexpr mat3 operator-(mat3 const& other) const noexcept
        {
            return mat3{ value - other.value };
        }

        [[nodiscard]] constexpr mat3 operator-(glm::mat3 const& other) const noexcept
        {
            return mat3{ value - other };
        }

        [[nodiscard]] constexpr mat3 operator-(vec3 const& other) const noexcept
        {
            glm::mat3 m = value;
            m -= other.data();
            return mat3{ m };
        }

        constexpr mat3& operator-=(float scalar) noexcept
        {
            value -= scalar;
            return *this;
        }

        constexpr mat3& operator-=(mat3 const& other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr mat3& operator-=(glm::vec3 const& other) noexcept
        {
            value -= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] vec3 operator*(vec3 const& v) const noexcept
        {
            return vec3{ value * v.data()};
        }

        [[nodiscard]] constexpr mat3 operator*(float scalar) const noexcept
        {
            return mat3{ value * scalar };
        }

        [[nodiscard]] constexpr mat3 operator*(mat3 const& other) const noexcept
        {
            return mat3{ value * other.value };
        }

        [[nodiscard]] constexpr mat3 operator*(glm::mat3 const& other) const noexcept
        {
            return mat3{ value * other };
        }

        constexpr mat3& operator*=(float scalar) noexcept
        {
            value *= scalar;
            return *this;
        }

        constexpr mat3& operator*=(mat3 const& other) noexcept
        {
            value *= other.value;
            return *this;
        }

        constexpr mat3& operator*=(glm::vec3 const& other) noexcept
        {
            value *= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Division
        // ---------------------------------------------------------------------------------

        [[nodiscard]] vec3 operator/(vec3 const& v) const noexcept
        {
            assert(!Math::anyZero(v.dataPtr(), 3));
            return vec3{ value / v.data() };
        }

        [[nodiscard]] constexpr mat3 operator/(float scalar) const noexcept
        {
            assert(!Math::isZero(scalar));
            return mat3{ value / scalar };
        }

        [[nodiscard]] constexpr mat3 operator/(mat3 const& other) const noexcept
        {
            assert(!Math::anyZero(other.dataPtr(), 9));
            return mat3{ value / other.value };
        }

        [[nodiscard]] constexpr mat3 operator/(glm::mat3 const& other) const noexcept
        {
            assert(!Math::anyZero(glm::value_ptr(other), 9));
            return mat3{ value / other };
        }

        constexpr mat3& operator/=(float scalar) noexcept
        {
            assert(!Math::isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr mat3& operator/=(mat3 const& other) noexcept
        {
            assert(!Math::anyZero(other.dataPtr(), 9));
            value /= other.value;
            return *this;
        }

        constexpr mat3& operator/=(glm::vec3 const& other) noexcept
        {
            assert(!Math::anyZero(glm::value_ptr(other), 3));
            value /= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Factory
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr static mat3 identity() noexcept
        {
            return mat3{ glm::mat3{ 1.0f } };
        }

        [[nodiscard]] constexpr static mat3 rotation(float angle, vec3 const& axis) noexcept
        {
            return mat3{ glm::mat3(glm::rotate(glm::mat4{1.0f}, angle, axis.data())) };
        }

        [[nodiscard]] static mat3 rotation(float xRad, float yRad, float zRad) noexcept
        {
            return mat3{ glm::mat3(glm::eulerAngleXYZ(xRad, yRad, zRad)) };
        }

        [[nodiscard]] constexpr static mat3 scaling(vec3 const& scale) noexcept
        {
            return mat3{ glm::mat3(glm::scale(glm::mat4{1.0f}, scale.data())) };
        }

        [[nodiscard]] constexpr static mat3 fromAxis(vec3 const& right, vec3 const& up, vec3 const& forward)
        {
            return mat3{ glm::mat3{ right.data(), up.data(), forward.data() } };
        }

        // ---------------------------------------------------------------------------------
        // Setters / Getters
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float* operator[](uint32_t col) noexcept
        {
            assert(col < 3);
            return dataPtr() + (col * 3);
        }

        [[nodiscard]] constexpr float const* operator[](uint32_t col) const noexcept
        {
            assert(col < 3);
            return dataPtr() + (col * 3);
        }

        constexpr void set(uint32_t col, uint32_t row, float v) noexcept
        {
            assert(col < 3);
            assert(row < 3);
            glm::value_ptr(value)[(col * 3) + row] = v;
        }

        constexpr void set(uint32_t index, float v) noexcept
        {
            assert(index < 9);
            glm::value_ptr(value)[index] = v;
        }

        [[nodiscard]] constexpr float get(uint32_t col, uint32_t row) const noexcept
        {
            assert(col < 3);
            assert(row < 3);
            return glm::value_ptr(value)[(col * 3) + row];
        }

        [[nodiscard]] constexpr float get(uint32_t index) const noexcept
        {
            assert(index < 9);
            return glm::value_ptr(value)[index];
        }

        constexpr void setCol(uint32_t col, float x, float y, float z) noexcept
        {
            assert(col < 3);
            set(col, 0, x);
            set(col, 1, y);
            set(col, 2, z);
        }

        constexpr void setCol(uint32_t col, vec3 const& v) noexcept
        {
            assert(col < 3);
            set(col, 0, v.x());
            set(col, 1, v.y());
            set(col, 2, v.z());
        }

        constexpr void setRow(uint32_t row, float x, float y, float z) noexcept
        {
            assert(row < 3);
            set(0, row, x);
            set(1, row, y);
            set(2, row, z);
        }

        constexpr void setRow(uint32_t row, vec3 const& v) noexcept
        {
            assert(row < 3);
            set(0, row, v.x());
            set(1, row, v.y());
            set(2, row, v.z());
        }

        [[nodiscard]] constexpr vec3 col(uint32_t col) const noexcept
        {
            assert(col < 3);
            return vec3{ get(col, 0), get(col, 1), get(col, 2) };
        }

        [[nodiscard]] constexpr vec3 row(uint32_t row) const noexcept
        {
            assert(row < 3);
            return vec3{ get(0, row), get(1, row), get(2, row) };
        }

        [[nodiscard]] constexpr vec3 right() const noexcept
        {
            return vec3{ value[0] }.normalized();
        }

        [[nodiscard]] constexpr vec3 up() const noexcept
        {
            return vec3{ value[1] }.normalized();
        }

        [[nodiscard]] constexpr vec3 forward() const noexcept
        {
            return vec3{ value[2] }.normalized();
        }

        [[nodiscard]] constexpr vec3 scale() const noexcept
        {
            return vec3{
                glm::length(value[0]),
                glm::length(value[1]),
                glm::length(value[2])
            };
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        constexpr mat3& zero() noexcept
        {
            value = { 0.0f };
            return *this;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return Math::allEquals(std::span{ dataPtr(), 9 }, 0.0f);
        }

        constexpr mat3& setIdentity() noexcept
        {
            value = { 1.0f };
            return *this;
        }

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return
                Math::isOne(get(0, 0))  && Math::isZero(get(0, 1)) && Math::isZero(get(0, 2)) &&
                Math::isZero(get(1, 0)) && Math::isOne(get(1, 1))  && Math::isZero(get(1, 2)) &&
                Math::isZero(get(2, 0)) && Math::isZero(get(2, 1)) && Math::isOne(get(2, 2));
        }

        constexpr mat3& transpose() noexcept
        {
            value = glm::transpose(value);
            return *this;
        }

        [[nodiscard]] constexpr mat3 transposed() const noexcept
        {
            return mat3{ glm::transpose(value) };
        }

        constexpr mat3& inverse() noexcept
        {
            value = glm::inverse(value);
            return *this;
        }

        [[nodiscard]] constexpr mat3 inverted() const noexcept
        {
            return mat3{ glm::inverse(value) };
        }

        [[nodiscard]] constexpr mat3 inverseTranspose() const noexcept
        {
            return mat3{ glm::transpose(glm::inverse(value)) };
        }

        [[nodiscard]] constexpr float determinant() const noexcept
        {
            return glm::determinant(value);
        }

        [[nodiscard]] constexpr vec3 transform(vec3 const& v) const noexcept
        {
            return (*this * v);
        }

        constexpr mat3& rotate(float angle, vec3 const& axis) noexcept
        {
            value = glm::rotate(glm::mat4{ value }, angle, axis.data());
            return *this;
        }

        constexpr mat3& scale(vec3 const& scaling) noexcept
        {
            value = glm::scale(glm::mat4{ value }, scaling.data());
            return *this;
        }

        std::string toString() const noexcept
        {
            return std::format("C0: [{:.3f},{:.3f},{:.3f}]\nC1: [{:.3f},{:.3f},{:.3f}]\nC2: [{:.3f},{:.3f},{:.3f}]",
                get(0, 0), get(0, 1), get(0, 2),
                get(1, 0), get(1, 1), get(1, 2),
                get(2, 0), get(2, 1), get(2, 2));
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::mat3& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::mat3 const& data() const noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr float* dataPtr() noexcept
        {
            return glm::value_ptr(value);
        }

        [[nodiscard]] constexpr float const* dataPtr() const noexcept
        {
            return glm::value_ptr(value);
        }

    private:

        friend struct mat4;

        glm::mat3 value{ 0.0f };
    };
}

REGISTER_TYPE_NAME(LITL::mat3)

// ... todo from quat when it is added ...

#endif