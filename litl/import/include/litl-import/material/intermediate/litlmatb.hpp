#ifndef LITL_IMPORT_MATERIAL_LITLMATB_H__
#define LITL_IMPORT_MATERIAL_LITLMATB_H__

#include <cstdint>
#include <string_view>

#include "litl-core/math/types.hpp"

namespace litl::import
{
    enum class LitlMatPropertyType : uint32_t
    {
        Unknown = 0u,
        Bool = 1u,
        Integer = 2u,
        UnsignedInteger = 3u,
        Float = 4u,
        Double = 5u,
        Vec2 = 6u,
        Vec3 = 7u,
        Vec4 = 8u,
        Color = 9u,
        Texture2D = 10u,
        Texture3D = 11u
    };

    struct LitlMatProperty
    {

    };

    class LitlMatBinary
    {
    public:

        void setName(std::string_view name) noexcept;
        void setBool(std::string_view property, bool value) noexcept;
        void setInt(std::string_view property, int32_t value) noexcept;
        void setUint(std::string_view property, uint32_t value) noexcept;
        void setFloat(std::string_view property, float value) noexcept;
        void setDouble(std::string_view property, double value) noexcept;
        void setVec2(std::string_view property, vec2 value) noexcept;
        void setVec3(std::string_view property, vec3 value) noexcept;
        void setVec4(std::string_view property, vec4 value) noexcept;
        void setColor(std::string_view property, color value) noexcept;
        void setTexture2D(std::string_view property, std::string_view resource) noexcept;
        void setTexture3D(std::string_view property, std::string_view resource) noexcept;

    private:
    };
}

#endif