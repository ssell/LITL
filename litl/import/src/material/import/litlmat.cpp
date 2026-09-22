#include <expected>
#include <format>
#include <glaze/toml.hpp>
#include <string>
#include <string_view>
#include <unordered_map>

#include "litl-core/string.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-import/material/import/litlmat.hpp"
#include "litl-import/material/intermediate/materialIntermediateData.hpp"

namespace litl::import
{
    namespace
    {
        static const std::unordered_map<std::string, LitlMatPropertyType> s_propertyTypeMap {
            { "bool",     LitlMatPropertyType::Bool },
            { "integer",  LitlMatPropertyType::Integer },
            { "int32",    LitlMatPropertyType::Integer },
            { "int",      LitlMatPropertyType::Integer },
            { "unsigned", LitlMatPropertyType::UnsignedInteger },
            { "uint32",   LitlMatPropertyType::UnsignedInteger },
            { "uint",     LitlMatPropertyType::UnsignedInteger },
            { "float",    LitlMatPropertyType::Float },
            { "double",   LitlMatPropertyType::Double },
            { "vec2",     LitlMatPropertyType::Vec2 },
            { "float2",   LitlMatPropertyType::Vec2 },
            { "vec3",     LitlMatPropertyType::Vec3 },
            { "float3",   LitlMatPropertyType::Vec3 },
            { "vec4",     LitlMatPropertyType::Vec4 },
            { "float4",   LitlMatPropertyType::Vec4 },
            { "color",    LitlMatPropertyType::Color },
            { "texture",  LitlMatPropertyType::Texture },
            { "tex2d",    LitlMatPropertyType::Texture }
        };

        static const std::unordered_map<std::string, LitlMatShaderStage> s_shaderTypeMap{
            { "vert",                   LitlMatShaderStage::Vertex },
            { "vertex",                 LitlMatShaderStage::Vertex },
            { "frag",                   LitlMatShaderStage::Fragment },
            { "fragment",               LitlMatShaderStage::Fragment },
            { "geo",                    LitlMatShaderStage::Geometry },
            { "geometry",               LitlMatShaderStage::Geometry },
            { "preTess",                LitlMatShaderStage::TessellationControl },
            { "preTessellation",        LitlMatShaderStage::TessellationControl },
            { "tessControl",            LitlMatShaderStage::TessellationControl },
            { "tessellationControl",    LitlMatShaderStage::TessellationControl },
            { "hull",                   LitlMatShaderStage::TessellationControl },
            { "postTess",               LitlMatShaderStage::TessellationEvaluation },
            { "postTessellation",       LitlMatShaderStage::TessellationEvaluation },
            { "tessEval",               LitlMatShaderStage::TessellationEvaluation },
            { "tessEvaluation",         LitlMatShaderStage::TessellationEvaluation },
            { "tessellationEval",       LitlMatShaderStage::TessellationEvaluation },
            { "tessellationEvaluation", LitlMatShaderStage::TessellationEvaluation },
            { "domain",                 LitlMatShaderStage::TessellationEvaluation },
            { "compute",                LitlMatShaderStage::Compute },
            { "mesh",                   LitlMatShaderStage::Mesh },
            { "task",                   LitlMatShaderStage::Task },
        };
    }

    // -------------------------------------------------------------------------------------
    // Material File Structure
    // -------------------------------------------------------------------------------------
    
    struct ShaderResourceStruct
    {
        std::string resource;
        std::string entry;
    };

    struct RasterSettingsStruct
    {
        std::string cullMode;
        std::string frontFace;
    };

    struct HintsStruct
    {
        bool frequentUpdates = false;
    };

    struct Property
    {
        std::string type;
        LitlMatSupportedRawPropertyTypes value;
    };

    using ShaderMap = std::unordered_map<std::string, ShaderResourceStruct>;
    using PropertyMap = std::unordered_map<std::string, Property>;

    struct ExpectedMaterialStructure
    {
        std::string name;
        ShaderMap shaders;
        RasterSettingsStruct raster;
        PropertyMap properties;
        HintsStruct hints;
    };

    // -------------------------------------------------------------------------------------
    // LitlMatImporter
    // -------------------------------------------------------------------------------------

    LitlMatImporter::LitlMatImporter()
    {

    }

    LitlMatImporter::~LitlMatImporter()
    {

    }

    bool importShadersTable(ExpectedMaterialStructure const& inputMaterial, MaterialIntermediateData* intermediateMaterial, std::string_view location) noexcept
    {
        for (auto& shaderKvp : inputMaterial.shaders)
        {
            const auto& shader = shaderKvp.second;
            const auto type = toLowercase(shaderKvp.first);

            if (shader.resource.empty())
            {
                logWarning(".litlmat import of ", location, ": shader '", type, "' missing required resource path ('resource'). Rejecting.");
                return false;
            }

            if (shader.entry.empty())
            {
                logWarning(".litlmat import of ", location, ": shader '", type, "' missing required entry point ('entry'). Rejecting.");
                return false;
            }

            const auto mappedType = s_shaderTypeMap.find(type);

            if (mappedType == s_shaderTypeMap.end())
            {
                logWarning(".litlmat import of ", location, ": shader '", type, "' invalid shader type '", type, "' specified. Rejecting.");
                return false;
            }

            if (mappedType->second != LitlMatShaderStage::Unknown)
            {
                if (!intermediateMaterial->setShader(mappedType->second, shader.resource, shader.entry))
                {
                    logWarning(".litlmat import of ", location, ": shader '", type, "' failed to be set. Rejecting.");
                    return false;
                }
            }
            else
            {
                logWarning(".litlmat import of ", location, ": shader '", type, "' has unhandled type of '", mappedType->first, "'. Rejecting.");
                return false;
            }
        }

        return true;
    }

    bool importRasterStateTable(ExpectedMaterialStructure const& inputMaterial, MaterialIntermediateData* intermediateMaterial, std::string_view location) noexcept
    {
        const auto cullMode = toLowercase(inputMaterial.raster.cullMode);

        if (cullMode.empty() || cullMode == "back")
        {
            intermediateMaterial->setRasterCullMode(LitlMatCullMode::Back);
        }
        else if (cullMode == "front")
        {
            intermediateMaterial->setRasterCullMode(LitlMatCullMode::Front);
        }
        else if (cullMode == "none")
        {
            intermediateMaterial->setRasterCullMode(LitlMatCullMode::None);
        }
        else if (cullMode == "both")
        {
            intermediateMaterial->setRasterCullMode(LitlMatCullMode::Both);
        }
        else
        {
            logWarning(".litlmat import of ", location, ": raster state 'cullMode' set to invalid value of '", cullMode, "'. Expected: 'back', 'front', 'none', 'both', or not provided. Rejecting.");
            return false;
        }

        const auto frontFace = toLowercase(inputMaterial.raster.frontFace);

        if (frontFace.empty() || frontFace == "clockwise" || frontFace == "cw")
        {
            intermediateMaterial->setRasterWinding(true);
        }
        else if (frontFace == "counterclockwise" || frontFace == "counter-clockwise" || frontFace == "ccw")
        {
            intermediateMaterial->setRasterWinding(false);
        }
        else
        {
            logWarning(".litlmat import of ", location, ": raster state 'frontFace' set to invalid value of '", cullMode, "'. Expected: 'clockwise', 'cw', 'counterclockwise', 'counter-clockwise', 'ccw', or not provided. Rejecting.");
            return false;
        }

        return true;
    }

    bool importPropertiesTable(ExpectedMaterialStructure const& inputMaterial, MaterialIntermediateData* intermediateMaterial, std::string_view location) noexcept
    {
        for (auto& propertyKvp : inputMaterial.properties)
        {
            const auto& property = propertyKvp.second;
            const auto type = toLowercase(property.type);

            if (type.empty())
            {
                logWarning(".litlmat import of ", location, ": property '", propertyKvp.first, "' missing type specifier. Rejecting.");
                return false;
            }

            const auto mappedType = s_propertyTypeMap.find(type);

            if (mappedType == s_propertyTypeMap.end())
            {
                logWarning(".litlmat import of ", location, ": property '", propertyKvp.first, "' invalid type '", type, "' specified. Rejecting.");
                return false;
            }

            if (!intermediateMaterial->addProperty(propertyKvp.first, mappedType->second, propertyKvp.second.value))
            {
                logWarning(".litlmat import of ", location, ": property '", propertyKvp.first, "' invalid data. Rejecting.");
                return false;
            }
        }

        return true;
    }

    bool importHintsTable(ExpectedMaterialStructure const& inputMaterial, MaterialIntermediateData* intermediateMaterial, std::string_view location) noexcept
    {
        intermediateMaterial->setHintFrequentUpdates(inputMaterial.hints.frequentUpdates);

        // ... add other hint types in the future ...

        return true;      // leave bool return to match other importX functions and for future compatibility.
    }

    Result LitlMatImporter::import(std::string_view location, std::span<std::byte const> sourceBytes, ImportSettings const& settings, ImportedData& importedData) noexcept
    {
        const auto sourceBytesString = std::string_view{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size() };

        ExpectedMaterialStructure inputMaterial{};
        const auto glzError = glz::read_toml(inputMaterial, sourceBytesString);

        if (glzError != glz::error_code::none)
        {
            return Result::Error(ErrorType::ImporterFailed, glzError.custom_error_message);
        }

        importedData.items.push_back({});
        auto& dataItem = importedData.items.back();

        if (!dataItem.setType(ImportedDataType::Material))
        {
            return Result::Error(ErrorType::ImporterFailed, "Failed to create material import data.");
        }

        auto* material = dataItem.getDataPtr<MaterialImportResult>();
        material->intermediateMaterial = std::make_unique<MaterialIntermediateData>();
        auto* intermediateMaterial = material->intermediateMaterial.get();

        if (!inputMaterial.name.empty())
        {
            intermediateMaterial->setName(inputMaterial.name);
        }
        else
        {
            intermediateMaterial->setName(location);
        }

        if (!importShadersTable(inputMaterial, intermediateMaterial, location))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [properties] table.", location));
        }

        if (!importRasterStateTable(inputMaterial, intermediateMaterial, location))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [raster] table.", location));
        }

        if (!importPropertiesTable(inputMaterial, intermediateMaterial, location))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [properties] table.", location));
        }

        if (!importHintsTable(inputMaterial, intermediateMaterial, location))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [hints] table.", location));
        }

        return Result::Success();
    }
}