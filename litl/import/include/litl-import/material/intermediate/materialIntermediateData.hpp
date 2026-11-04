#ifndef LITL_IMPORT_MATERIAL_INTERMEDIATE_DATA_H__
#define LITL_IMPORT_MATERIAL_INTERMEDIATE_DATA_H__

#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
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
        Texture = 10u
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
        Task = 8u,
        StageCount
    };

    enum class LitlMatCullMode : uint32_t
    {
        Back = 0u,
        Front = 1u,
        None = 2u,
        Both = 3u
    };

    /// <summary>
    /// Must match MaterialBlendMode in litl-engine/objects/material/material.hpp
    /// </summary>
    enum class LitlMatBlendMode : uint32_t
    {
        Opaque      = 0u,
        Masked      = 1u,
        Transparent = 2u
    };

    /// <summary>
    /// Must match BlendOperationType in litl-renderer/enums.hpp
    /// </summary>
    enum class LitlMatBlendOperation : uint32_t
    {
        Add             = 0u,
        Subtract        = 1u,
        ReverseSubtract = 2u,
        Min             = 3u,
        Max             = 4u
    };

    /// <summary>
    /// Must match BlendFactor in litl-renderer/enums.hpp
    /// </summary>
    enum class LitlMatBlendFactor : uint32_t
    {
        Zero                  = 0u,
        One                   = 1u,
        SrcColor              = 2u,
        OneMinusSrcColor      = 3u,
        DstColor              = 4u,
        OneMinusDstColor      = 5u,
        SrcAlpha              = 6u,
        OneMinusSrcAlpha      = 7u,
        DstAlpha              = 8u,
        OneMinusDstAlpha      = 9u,
        ConstantColor         = 10u,
        OneMinusConstantColor = 11u,
        ConstantAlpha         = 12u,
        OneMinusConstantAlpha = 13u,
        SrcAlphaSaturate      = 14u
    };

    /// <summary>
    /// Enum identifiers for the blend mode presets to avoid having to store strings in the binary.
    /// </summary>
    enum class LitlMatBlendPreset : uint32_t
    {
        None          = 0u,
        Alpha         = 1u,
        Premultiplied = 2u,
        Additive      = 3u,
        AdditiveAlpha = 4u,
        Multiply      = 5u,
        Multiply2x    = 6u,
        Screen        = 7u,
        Min           = 8u,
        Max           = 9u
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

        // Blend
        LitlMatBlendMode blendMode{ LitlMatBlendMode::Opaque };
        LitlMatBlendPreset blendPreset{ LitlMatBlendPreset::None };
        LitlMatBlendFactor srcColor{ LitlMatBlendFactor::Zero };
        LitlMatBlendFactor dstColor{ LitlMatBlendFactor::Zero };
        LitlMatBlendOperation colorBlendOp{ LitlMatBlendOperation::Add };
        LitlMatBlendFactor srcAlpha{ LitlMatBlendFactor::Zero };
        LitlMatBlendFactor dstAlpha{ LitlMatBlendFactor::Zero };
        LitlMatBlendOperation alphaBlendOp{ LitlMatBlendOperation::Add };

        // Hints
        bool frequentUpdates{ false };
    };

    class MaterialIntermediateData
    {
    public:

        [[nodiscard]] bool setShader(LitlMatShaderStage stage, std::string const& resource, std::string const& entry) noexcept;
        [[nodiscard]] bool addProperty(std::string const& name, LitlMatPropertyType type, LitlMatSupportedRawPropertyTypes const& value) noexcept;
        [[nodiscard]] bool setProperty(std::string const& name, LitlMatPropertyType type, LitlMatSupportedRawPropertyTypes const& value) noexcept;

        void setName(std::string_view name) noexcept;
        void setRasterCullMode(LitlMatCullMode cullMode) noexcept;
        void setRasterWinding(bool clockwise) noexcept;
        void setBlendMode(LitlMatBlendMode blendMode) noexcept;
        void setBlendPreset(LitlMatBlendPreset blendPreset) noexcept;
        void setBlendSrcColor(LitlMatBlendFactor factor) noexcept;
        void setBlendDstColor(LitlMatBlendFactor factor) noexcept;
        void setBlendColorOp(LitlMatBlendOperation op) noexcept;
        void setBlendSrcAlpha(LitlMatBlendFactor factor) noexcept;
        void setBlendDstAlpha(LitlMatBlendFactor factor) noexcept;
        void setBlendAlphaOp(LitlMatBlendOperation op) noexcept;
        void setHintFrequentUpdates(bool frequentUpdates) noexcept;

        /// <summary>
        /// Returns an array of shaders used by the material. Indexed by the LitlMatShaderStage.
        /// </summary>
        [[nodiscard]] std::array<LitlMatShaderRecord, static_cast<uint32_t>(LitlMatShaderStage::StageCount)> const& getShaders() const noexcept;

        /// <summary>
        /// Returns all material properties.
        /// </summary>
        [[nodiscard]] std::vector<LitlMatPropertyRecord> const& getProperties() const noexcept;

        /// <summary>
        /// Returns the property with the specified case-sensitive name.
        /// </summary>
        [[nodiscard]] std::optional<LitlMatPropertyRecord> getProperty(std::string_view name) const noexcept;

        /// <summary>
        /// Returns the material settings.
        /// </summary>
        [[nodiscard]] LitlMatSettings const& getSettings() const noexcept;

    private:

        std::array<LitlMatShaderRecord, static_cast<uint32_t>(LitlMatShaderStage::StageCount)> m_shaders;
        std::vector<LitlMatPropertyRecord> m_properties;
        LitlMatSettings m_settings;
    };

    static_assert(std::is_move_constructible_v<MaterialIntermediateData>);
    static_assert(std::is_move_assignable_v<MaterialIntermediateData>);
}

#endif