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
        Back = 0u,
        Front = 1u,
        None = 2u,
        Both = 3u
    };

    /// <summary>
    /// Supported types to come over directly from an external file.
    /// </summary>
    using LitlMatSupportedRawPropertyTypes = std::variant<
        bool,
        uint8_t,        // bool alternative (and what it is stored as in the binary file)
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

    /// <summary>
    /// Supported types allowed to be stored directly in the intermediate data.
    /// The difference between LitlMatSupportedRawPropertyTypes is the lack of generic vector support.
    /// </summary>
    using LitlMatSupportedPropertyTypes = std::variant<
        uint8_t,
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
        std::string name;
        LitlMatPropertyType type{ LitlMatPropertyType::Unknown };
        LitlMatSupportedPropertyTypes value;
    };

    struct LitlMatSettings
    {
        // General
        std::string materialName;

        // Raster
        LitlMatCullMode cullMode{ LitlMatCullMode::Back };
        bool clockwise{ true };

        // Hints
        bool frequentUpdates{ false };
    };

    class MaterialIntermediateData
    {
    public:

        static constexpr uint32_t ShaderStageCount = 8u;

        [[nodiscard]] bool setShader(LitlMatShaderStage stage, std::string const& resource, std::string const& entry) noexcept;
        [[nodiscard]] bool addProperty(std::string const& name, LitlMatPropertyType type, LitlMatSupportedRawPropertyTypes const& value) noexcept;

        void setName(std::string_view name) noexcept;
        void setRasterCullMode(LitlMatCullMode cullMode) noexcept;
        void setRasterWinding(bool clockwise) noexcept;
        void setHintFrequentUpdates(bool frequentUpdates) noexcept;

        std::array<LitlMatShaderRecord, 8> const& getShaders() const noexcept;
        std::vector<LitlMatPropertyRecord> const& getProperties() const noexcept;
        LitlMatSettings const& getSettings() const noexcept;

    private:

        std::array<LitlMatShaderRecord, ShaderStageCount> m_shaders;
        std::vector<LitlMatPropertyRecord> m_properties;
        LitlMatSettings m_settings;;
    };
}

#endif