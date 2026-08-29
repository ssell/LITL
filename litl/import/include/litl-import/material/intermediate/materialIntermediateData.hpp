#ifndef LITL_IMPORT_MATERIAL_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_MATERIAL_INTERMEDIATE_DATA_H__

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>

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

    enum class LitlMatShaderStage : uint32_t
    {
        Unknown = 0u,
        Vertex = 1u,
        Fragment = 2u,
        Geometry = 3u,
        TessellationControl = 4u,
        TessellationEvaluation = 5u,
        Compute = 6u,
        Mesh = 7u,
        Task = 8u
    };

    enum class LitlMatCullMode : uint32_t
    {
        Front = 0u,
        Back = 1u,
        None = 2u,
        Both = 3u
    };

    using LitlMatSupportedRawPropertyTypes = std::variant<
        bool,
        int32_t,
        uint32_t,
        float,
        double,
        std::string,
        std::vector<float>,
        vec2,
        vec3,
        vec4,
        color
    >;

    using LitlMatSupportedPropertyTypes = std::variant<
        bool,
        int32_t,
        uint32_t,
        float,
        double,
        std::string,
        vec2,
        vec3,
        vec4,
        color
    >;

    struct LitlMatShaderRecord
    {
        LitlMatShaderStage stage{ LitlMatShaderStage::Unknown };
        std::string resource;
        std::string entry;
    };

    struct LitlMatPropertyRecord
    {
        LitlMatPropertyType type{ LitlMatPropertyType::Unknown };
        LitlMatSupportedPropertyTypes value;
    };

    struct LitlMatRasterSettings
    {
        LitlMatCullMode cullMode{ LitlMatCullMode::Back };
        bool clockwise{ true };
    };

    struct LitlMatHintSettings
    {
        bool frequentUpdates{ false };
    };

    class MaterialIntermediateData
    {
    public:

        void setName(std::string_view name) noexcept;
        [[nodiscard]] bool setShader(LitlMatShaderStage stage, std::string const& resource, std::string const& entry) noexcept;
        [[nodiscard]] bool addProperty(LitlMatPropertyType type, LitlMatSupportedRawPropertyTypes const& value) noexcept;
        void setRasterCullMode(LitlMatCullMode cullMode) noexcept;
        void setRasterWinding(bool clockwise) noexcept;
        void setHintFrequentUpdates(bool frequentUpdates) noexcept;

        std::array<LitlMatShaderRecord, 7> const& getShaders() const noexcept;
        std::vector<LitlMatPropertyRecord> const& getProperties() const noexcept;
        LitlMatRasterSettings const& getRasterSettings() const noexcept;
        LitlMatHintSettings const& getHintSettings() const noexcept;

    private:

        std::array<LitlMatShaderRecord, 7> m_shaders;
        std::vector<LitlMatPropertyRecord> m_properties;
        LitlMatRasterSettings m_rasterSettings;
        LitlMatHintSettings m_hintSettings;
    };
}

#endif