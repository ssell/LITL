#ifndef LITL_MATH_MAT4_H__
#define LITL_MATH_MAT4_H__

#include <cassert>
#include <format>
#include <span>
#include <string>

#include "litl-core/math/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_relational.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "litl-core/types.hpp"
#include "litl-core/math/common.hpp"
#include "litl-core/math/types/vec4.hpp"
#include "litl-core/math/types/vec3.hpp"
#include "litl-core/math/types/mat3.hpp"

namespace litl
{
    struct quat;

    /// <summary>
    /// Column-major: [col][row], (mat * vec), etc.
    /// Reverse-order multiplication. For example:
    /// 
    ///     mat4 model = translation * rotation * scale;
    /// 
    /// Applies scale, then rotation, then finally translation.
    /// 
    ///     mat4 mvp = projection * view * model;
    ///     vec4 worldPos = mvp * localPos;
    /// 
    /// Memory is stored as: [col0.x, col0.y, col0.z, col0.w, col1.x, ...]
    /// </summary>
    struct mat4
    {
        constexpr mat4() {}
        constexpr mat4(mat4 const& other) : value(other.value) {}
        constexpr explicit mat4(glm::mat4 const& other) : value(other) {}
        explicit mat4(mat3 const& other);
        explicit mat4(quat const& quaternion);

        constexpr explicit mat4(std::span<float const> values)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(values.size()) && (i < 16); ++i)
            {
                set(i, values[i]);
            }
        }

        // ---------------------------------------------------------------------------------
        // Equality
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr bool operator==(mat4 const& other) const noexcept
        {
            return glm::all(glm::equal(value, other.value, Traits<float>::epsilon));
        }

        [[nodiscard]] constexpr bool operator==(glm::mat4 const& other) const noexcept
        {
            return glm::all(glm::equal(value, other, Traits<float>::epsilon));
        }

        // ---------------------------------------------------------------------------------
        // Negation
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat4 operator-() const noexcept
        {
            return mat4{ -value };
        }

        // ---------------------------------------------------------------------------------
        // Addition
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat4 operator+(float scalar) const noexcept
        {
            return mat4{ value + scalar };
        }

        [[nodiscard]] constexpr mat4 operator+(mat4 const& other) const noexcept
        {
            return mat4{ value + other.value };
        }

        [[nodiscard]] constexpr mat4 operator+(glm::mat4 const& other) const noexcept
        {
            return mat4{ value + other };
        }

        [[nodiscard]] constexpr mat4 operator+(vec4 const& other) const noexcept
        {
            glm::mat4 m = value;
            m += other.data();
            return mat4{ m };
        }

        constexpr mat4& operator+=(float scalar) noexcept
        {
            value += scalar;
            return *this;
        }

        constexpr mat4& operator+=(mat4 const& other) noexcept
        {
            value += other.value;
            return *this;
        }

        constexpr mat4& operator+=(glm::vec4 const& other) noexcept
        {
            value += other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Subtraction
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr mat4 operator-(float scalar) const noexcept
        {
            return mat4{ value - scalar };
        }

        [[nodiscard]] constexpr mat4 operator-(mat4 const& other) const noexcept
        {
            return mat4{ value - other.value };
        }

        [[nodiscard]] constexpr mat4 operator-(glm::mat4 const& other) const noexcept
        {
            return mat4{ value - other };
        }

        [[nodiscard]] constexpr mat4 operator-(vec4 const& other) const noexcept
        {
            glm::mat4 m = value;
            m -= other.data();
            return mat4{ m };
        }

        constexpr mat4& operator-=(float scalar) noexcept
        {
            value -= scalar;
            return *this;
        }

        constexpr mat4& operator-=(mat4 const& other) noexcept
        {
            value -= other.value;
            return *this;
        }

        constexpr mat4& operator-=(glm::vec4 const& other) noexcept
        {
            value -= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Multiplication
        // ---------------------------------------------------------------------------------

        [[nodiscard]] vec3 operator*(vec3 const& v) const noexcept
        {
            return vec3{ value * glm::vec4(v.x(), v.y(), v.z(), 1.0f )};
        }

        [[nodiscard]] vec4 operator*(vec4 const& v) const noexcept
        {
            return vec4{ value * v.data() };
        }

        [[nodiscard]] constexpr mat4 operator*(float scalar) const noexcept
        {
            return mat4{ value * scalar };
        }

        [[nodiscard]] constexpr mat4 operator*(mat4 const& other) const noexcept
        {
            return mat4{ value * other.value };
        }

        [[nodiscard]] constexpr mat4 operator*(glm::mat4 const& other) const noexcept
        {
            return mat4{ value * other };
        }

        constexpr mat4& operator*=(float scalar) noexcept
        {
            value *= scalar;
            return *this;
        }

        constexpr mat4& operator*=(mat4 const& other) noexcept
        {
            value *= other.value;
            return *this;
        }

        constexpr mat4& operator*=(glm::vec4 const& other) noexcept
        {
            value *= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Division
        // ---------------------------------------------------------------------------------

        [[nodiscard]] vec3 operator/(vec3 const& v) const noexcept
        {
            assert(!anyZero(v.dataPtr(), 3));
            return vec3{ value / glm::vec4(v.x(), v.y(), v.z(), 1.0f) };
        }

        [[nodiscard]] vec4 operator/(vec4 const& v) const noexcept
        {
            assert(!anyZero(v.dataPtr(), 4));
            return vec4{ value / v.data() };
        }

        [[nodiscard]] constexpr mat4 operator/(float scalar) const noexcept
        {
            assert(!isZero(scalar));
            return mat4{ value / scalar };
        }

        [[nodiscard]] constexpr mat4 operator/(mat4 const& other) const noexcept
        {
            assert(!anyZero(other.dataPtr(), 16));
            return mat4{ value / other.value };
        }

        [[nodiscard]] constexpr mat4 operator/(glm::mat4 const& other) const noexcept
        {
            assert(!anyZero(glm::value_ptr(other), 16));
            return mat4{ value / other };
        }

        constexpr mat4& operator/=(float scalar) noexcept
        {
            assert(!isZero(scalar));
            value /= scalar;
            return *this;
        }

        constexpr mat4& operator/=(mat4 const& other) noexcept
        {
            assert(!anyZero(other.dataPtr(), 16));
            value /= other.value;
            return *this;
        }

        constexpr mat4& operator/=(glm::vec4 const& other) noexcept
        {
            assert(!anyZero(glm::value_ptr(other), 4));
            value /= other;
            return *this;
        }

        // ---------------------------------------------------------------------------------
        // Factory
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr static mat4 identity() noexcept
        {
            return mat4{ glm::mat4{ 1.0f } };
        }

        [[nodiscard]] constexpr static mat4 translation(vec3 const& v) noexcept
        {
            return mat4{ glm::translate(glm::mat4{1.0f}, v.data()) };
        }

        [[nodiscard]] constexpr static mat4 translation(vec4 const& v) noexcept
        {
            return mat4{ glm::translate(glm::mat4{1.0f}, v.xyz().data()) };
        }

        [[nodiscard]] constexpr static mat4 rotation(float angle, vec3 const& axis) noexcept
        {
            return mat4{ glm::rotate(glm::mat4{1.0f}, angle, axis.data()) };
        }

        [[nodiscard]] static mat4 rotation(float xRad, float yRad, float zRad) noexcept
        {
            return mat4{ glm::mat4(glm::mat3(glm::eulerAngleXYZ(xRad, yRad, zRad))) };
        }

        [[nodiscard]] constexpr static mat4 scaling(vec3 const& scale) noexcept
        {
            return mat4{ glm::scale(glm::mat4{1.0f}, scale.data()) };
        }

        [[nodiscard]] constexpr static mat4 lookAt(vec3 const& cameraPos, vec3 const& targetPos, vec3 const& up) noexcept
        {
            return mat4{ glm::lookAtLH(cameraPos.data(), targetPos.data(), up.data()) };
        }

        /// <summary>
        /// Constructs a perspective projection matrix with the following properties:<br/>
        /// 
        ///     * Left-handed: (+x = right, +y = up, +z = forward<br/>
        ///     * Clip-space Z on the range [0, 1] = [far, near]<br/>
        /// </summary>
        /// <param name="fovY"></param>
        /// <param name="aspect"></param>
        /// <param name="zNear"></param>
        /// <param name="zFar"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr static mat4 perspective(float fovY, float aspect, float zNear, float zFar) noexcept
        {
            // Note below we are intentionally flipping the near and flar clip. such that far = 0 and near = w. In floating point numbers, a greater
            // amount of precision is given to those numbers closer to zero. By flipping far and near we gain precision at the far clip and reduce artifacts.
            return mat4{ glm::perspectiveLH(fovY, aspect, zFar, zNear) };
        }

        /// <summary>
        /// Constructs an orthographic projection matrix with the following properties:<br/>
        /// 
        ///     * Left-handed: (+x = right, +y = up, +z = forward<br/>
        ///     * Clip-space Z on the range [0, 1] = [far, near]<br/>
        /// </summary>
        /// <param name="fovY"></param>
        /// <param name="aspect"></param>
        /// <param name="zNear"></param>
        /// <param name="zFar"></param>
        /// <returns></returns>
        [[nodiscard]] constexpr static mat4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar) noexcept
        {
            // Note below we are intentionally flipping the near and flar clip. such that far = 0 and near = w. In floating point numbers, a greater
            // amount of precision is given to those numbers closer to zero. By flipping far and near we gain precision at the far clip and reduce artifacts.
            return mat4{ glm::orthoLH(left, right, bottom, top, zFar, zNear) };
        }

        [[nodiscard]] constexpr static mat4 fromAxis(vec4 const& right, vec4 const& up, vec4 const& forward, vec4 const& position)
        {
            return mat4{ glm::mat4{ right.data(), up.data(), forward.data(), position.data() } };
        }

        // ---------------------------------------------------------------------------------
        // Setters / Getters
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr float* operator[](uint32_t col) noexcept
        {
            assert(col < 4);
            return dataPtr() + (col * 4);
        }

        [[nodiscard]] constexpr float const* operator[](uint32_t col) const noexcept
        {
            assert(col < 4);
            return dataPtr() + (col * 4);
        }

        constexpr void set(uint32_t col, uint32_t row, float v) noexcept
        {
            assert(col < 4);
            assert(row < 4);
            value[col][row] = v;
        }

        constexpr void set(uint32_t index, float v) noexcept
        {
            assert(index < 16);
            glm::value_ptr(value)[index] = v;
        }

        [[nodiscard]] constexpr float get(uint32_t col, uint32_t row) const noexcept
        {
            assert(col < 4);
            assert(row < 4);
            return value[col][row];
        }

        [[nodiscard]] constexpr float get(uint32_t index) const noexcept
        {
            assert(index < 16);
            return glm::value_ptr(value)[index];
        }

        constexpr void setCol(uint32_t col, float x, float y, float z, float w) noexcept
        {
            assert(col < 4);
            set(col, 0, x);
            set(col, 1, y);
            set(col, 2, z);
            set(col, 3, w);
        }

        constexpr void setCol(uint32_t col, vec4 const& v) noexcept
        {
            assert(col < 4);
            set(col, 0, v.x());
            set(col, 1, v.y());
            set(col, 2, v.z());
            set(col, 3, v.w());
        }

        constexpr void setRow(uint32_t row, float x, float y, float z, float w) noexcept
        {
            assert(row < 4);
            set(0, row, x);
            set(1, row, y);
            set(2, row, z);
            set(3, row, w);
        }

        constexpr void setRow(uint32_t row, vec4 const& v) noexcept
        {
            assert(row < 4);
            set(0, row, v.x());
            set(1, row, v.y());
            set(2, row, v.z());
            set(3, row, v.w());
        }

        [[nodiscard]] constexpr vec4 col(uint32_t col) const noexcept
        {
            assert(col < 4);
            return vec4{ value[col][0], value[col][1], value[col][2], value[col][3] };
        }

        [[nodiscard]] constexpr vec4 row(uint32_t row) const noexcept
        {
            assert(row < 4);
            return vec4{ value[0][row], value[1][row], value[2][row], value[3][row] };
        }

        [[nodiscard]] constexpr vec4 translation() const noexcept
        {
            return vec4{ value[3] };
        }

        [[nodiscard]] constexpr vec3 right() const noexcept
        {
            return vec3{ value[0][0], value[0][1], value[0][2] }.normalized();
        }

        [[nodiscard]] constexpr vec3 up() const noexcept
        {
            return vec3{ value[1][0], value[1][1], value[1][2] }.normalized();
        }

        [[nodiscard]] constexpr vec3 forward() const noexcept
        {
            return vec3{ value[2][0], value[2][1], value[2][2] }.normalized();
        }

        [[nodiscard]] constexpr vec3 scale() const noexcept
        {
            return vec3{
                glm::length(glm::vec3{ value[0] }),
                glm::length(glm::vec3{ value[1] }),
                glm::length(glm::vec3{ value[2] })
            };
        }

        [[nodiscard]] constexpr mat3 get3x3() const noexcept
        {
            return mat3{ glm::mat3(value) };
        }

        // ---------------------------------------------------------------------------------
        // Utility
        // ---------------------------------------------------------------------------------

        constexpr mat4& zero() noexcept
        {
            value = { 0.0f };
            return *this;
        }

        [[nodiscard]] constexpr bool isZeroed() const noexcept
        {
            return allEquals(std::span{ dataPtr(), 16 }, 0.0f);
        }

        constexpr mat4& setIdentity() noexcept
        {
            value = { 1.0f };
            return *this;
        }

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return
                isOne(get(0, 0)) && isZero(get(0, 1)) && isZero(get(0, 2)) && isZero(get(0, 3)) &&
                isZero(get(1, 0)) && isOne(get(1, 1)) && isZero(get(1, 2)) && isZero(get(1, 3)) &&
                isZero(get(2, 0)) && isZero(get(2, 1)) && isOne(get(2, 2)) && isZero(get(2, 3)) &&
                isZero(get(3, 0)) && isZero(get(3, 1)) && isZero(get(3, 2)) && isOne(get(3, 3));
        }

        constexpr mat4& transpose() noexcept
        {
            value = glm::transpose(value);
            return *this;
        }

        [[nodiscard]] constexpr mat4 transposed() const noexcept
        {
            return mat4{ glm::transpose(value) };
        }

        constexpr mat4& inverse() noexcept
        {
            value = glm::inverse(value);
            return *this;
        }

        [[nodiscard]] constexpr mat4 inverted() const noexcept
        {
            return mat4{ glm::inverse(value) };
        }

        [[nodiscard]] constexpr mat4 inverseTranspose() const noexcept
        {
            return mat4{ glm::transpose(glm::inverse(value)) };
        }

        [[nodiscard]] constexpr float determinant() const noexcept
        {
            return glm::determinant(value);
        }

        [[nodiscard]] constexpr vec3 transform(vec3 const& v) const noexcept
        {
            return (*this * v);
        }

        [[nodiscard]] constexpr vec4 transform(vec4 const& v) const noexcept
        {
            return (*this * v);
        }

        [[nodiscard]] constexpr vec3 transformDirection(vec3 const& normal) const noexcept
        {
            return vec3(glm::mat3(value) * normal.data());
        }

        constexpr mat4& translate(vec4 const& v) noexcept
        {
            value = glm::translate(value, glm::vec3{ v.x(), v.y(), v.z() });
            return *this;
        }

        constexpr mat4& translate(vec3 const& v) noexcept
        {
            value = glm::translate(value, v.data());
            return *this;
        }

        constexpr mat4& rotate(float angle, vec3 const& axis) noexcept
        {
            value = glm::rotate(value, angle, axis.data());
            return *this;
        }

        constexpr mat4& scale(vec3 const& scaling) noexcept
        {
            value = glm::scale(value, scaling.data());
            return *this;
        }

        std::string toString() const noexcept
        {
            return std::format("C0: [{:.3f},{:.3f},{:.3f},{:.3f}]\nC1: [{:.3f},{:.3f},{:.3f},{:.3f}]\nC2: [{:.3f},{:.3f},{:.3f},{:.3f}]\nC3: [{:.3f},{:.3f},{:.3f},{:.3f}]",
                get(0, 0), get(0, 1), get(0, 2), get(0, 3),
                get(1, 0), get(1, 1), get(1, 2), get(1, 3),
                get(2, 0), get(2, 1), get(2, 2), get(2, 3),
                get(3, 0), get(3, 1), get(3, 2), get(3, 3));
        }

        // ---------------------------------------------------------------------------------
        // Access
        // ---------------------------------------------------------------------------------

        [[nodiscard]] constexpr glm::mat4& data() noexcept
        {
            return value;
        }

        [[nodiscard]] constexpr glm::mat4 const& data() const noexcept
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

        friend struct mat3;

        glm::mat4 value{ 0.0f };
    };
}

REGISTER_TYPE_NAME(litl::mat4)

#endif