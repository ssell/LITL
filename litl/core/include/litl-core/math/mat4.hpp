#ifndef LITL_MATH_MAT4_H__
#define LITL_MATH_MAT4_H__

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <initializer_list>

#include "litl-core/math/math.hpp"
#include "litl-core/math/vec3.hpp"
#include "litl-core/math/vec4.hpp"
#include "litl-core/types.hpp"

namespace LITL::Math
{
    /*
    template<typename T> 
    struct Mat4Wrapper
    {
        glm::mat4 value{1.0f};

        constexpr Mat4Wrapper() = default;
        constexpr explicit Mat4Wrapper(Mat4Wrapper const& other) : value(other.value) {}
        constexpr explicit Mat4Wrapper(glm::mat4 v) : value(v) {}

        constexpr Mat4Wrapper(std::initializer_list<float> values)
        {
            uint32_t i = 0;

            for (auto& value : values)
            {
                if (i >= 16)
                {
                    break;
                }

                set(i, value);
            }
        }

        Mat4Wrapper& operator=(glm::mat4 const& other) noexcept
        {
            value = other;
            return *this;
        }

        Mat4Wrapper& operator=(Mat4Wrapper const& other) noexcept
        {
            value = other.value;
            return *this;
        }

        void set(uint32_t i, float x) noexcept
        {
            assert(i < 16);
            value[i / 4][i % 4] = x;
        }

        void set(uint32_t col, uint32_t row, float x) noexcept
        {
            assert((col < 4) && (row < 4));
            value[col][row] = x;
        }

        void setRow(uint32_t row, float x, float y, float z, float w) noexcept
        {
            assert(row < 4);
            set(0, row, x);
            set(1, row, y);
            set(2, row, z);
            set(3, row, w);
        }

        void setCol(uint32_t col, float x, float y, float z, float w) noexcept
        {
            assert(col < 4);
            set(col, 0, x);
            set(col, 1, y);
            set(col, 2, z);
            set(col, 3, w);
        }

        [[nodiscard]] float get(uint32_t col, uint32_t row) const noexcept
        {
            return value[col][row];
        }

        void identity() noexcept
        {
            value[0][0] = 1.0f; value[1][0] = 0.0f; value[2][0] = 0.0f; value[3][0] = 0.0f;
            value[0][1] = 0.0f; value[1][1] = 1.0f; value[2][1] = 0.0f; value[3][1] = 0.0f;
            value[0][2] = 0.0f; value[1][2] = 0.0f; value[2][2] = 1.0f; value[3][2] = 0.0f;
            value[0][3] = 0.0f; value[1][3] = 0.0f; value[2][3] = 0.0f; value[3][3] = 1.0f;
        }

        [[nodiscard]] constexpr bool isIdentity() const noexcept
        {
            return
                fequals(get(0, 0), 1.0f) && fequals(get(1, 0), 0.0f) && fequals(get(2, 0), 0.0f) && fequals(get(3, 0), 0.0f) &&
                fequals(get(0, 1), 0.0f) && fequals(get(1, 1), 1.0f) && fequals(get(2, 1), 0.0f) && fequals(get(3, 1), 0.0f) &&
                fequals(get(0, 2), 0.0f) && fequals(get(1, 2), 0.0f) && fequals(get(2, 2), 1.0f) && fequals(get(3, 2), 0.0f) &&
                fequals(get(0, 3), 0.0f) && fequals(get(1, 3), 0.0f) && fequals(get(2, 3), 0.0f) && fequals(get(3, 3), 1.0f);
        }

        void zero() noexcept
        {
            value[0][0] = 0.0f; value[1][0] = 0.0f; value[2][0] = 0.0f; value[3][0] = 0.0f;
            value[0][1] = 0.0f; value[1][1] = 0.0f; value[2][1] = 0.0f; value[3][1] = 0.0f;
            value[0][2] = 0.0f; value[1][2] = 0.0f; value[2][2] = 0.0f; value[3][2] = 0.0f;
            value[0][3] = 0.0f; value[1][3] = 0.0f; value[2][3] = 0.0f; value[3][3] = 0.0f;
        }

        [[nodiscard]] constexpr bool isZero() const noexcept
        {
            return
                Math::isZero(get(0, 0)) && Math::isZero(get(1, 0)) && Math::isZero(get(2, 0)) && Math::isZero(get(3, 0)) &&
                Math::isZero(get(0, 1)) && Math::isZero(get(1, 1)) && Math::isZero(get(2, 1)) && Math::isZero(get(3, 1)) &&
                Math::isZero(get(0, 2)) && Math::isZero(get(1, 2)) && Math::isZero(get(2, 2)) && Math::isZero(get(3, 2)) &&
                Math::isZero(get(0, 3)) && Math::isZero(get(1, 3)) && Math::isZero(get(2, 3)) && Math::isZero(get(3, 3));
        }

        void transpose() noexcept
        {
            value = glm::transpose(value);
        }

        [[nodiscard]] Mat4Wrapper transposed() const noexcept
        {
            return glm::transpose(value);
        }

        void inverse() noexcept
        {
            value = glm::inverse(value);
        }

        [[nodiscard]] Mat4Wrapper inverted() const noexcept
        {
            return glm::inverse(value);
        }

        [[nodiscard]] float determinant() const noexcept
        {
            return glm::determinant(value);
        }

        [[nodiscard]] constexpr static Mat4Wrapper translation(Vec3 const& v) noexcept
        {
            // {1.0f} == shorthand for identity matrix
            return glm::translate({ 1.0f }, v.value);
        }

        [[nodiscard]] constexpr static Mat4Wrapper rotation(float angle, Vec3 const& axis) noexcept
        {
            // {1.0f} == shorthand for identity matrix
            return glm::rotate({ 1.0f }, axis.value);
        }

        [[nodiscard]] constexpr static Mat4Wrapper scaling(Vec3 const& scaling) noexcept
        {
            // {1.0f} == shorthand for identity matrix
            return glm::scale({ 1.0f }, scaling.value);
        }

        [[nodiscard]] constexpr static Mat4Wrapper lookAt(Vec3 const& cameraPos, Vec3 const& targetPos, Vec3 const& up) noexcept
        {
            return glm::lookAt(cameraPos.value, targetPos.value, up.value);
        }

        [[nodiscard]] constexpr static Mat4Wrapper perspective(float fovY, float aspect, float zNear, float zFar) noexcept
        {
            return glm::perspective(fovY, aspect, zNear, zFar);
        }
    };

    struct Mat4 : Mat4Wrapper<Mat4>
    {
        using Mat4Wrapper::Mat4Wrapper;
    };
    */
}

//REGISTER_TYPE_NAME(LITL::Math::Mat4);

#endif