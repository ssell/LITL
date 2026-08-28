#include <expected>
#include <format>
#include <glaze/toml.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>

#include "litl-core/string.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-import/material/import/litlmat.hpp"
#include "litl-import/material/intermediate/litlmatb.hpp"

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
            { "texture",  LitlMatPropertyType::Texture2D },
            { "tex2d",    LitlMatPropertyType::Texture2D },
            { "tex3d",    LitlMatPropertyType::Texture3D }
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
    
    // The below structures can not be in the empty namespace or else they are not visible to glaze during deserialziation.
    using SupportedPropertyTypes = std::variant<
        bool,
        int32_t,
        uint32_t,
        float,
        std::string,
        std::vector<float>
    >;

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
        SupportedPropertyTypes value;
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

    bool importShadersTable(ExpectedMaterialStructure const& inputMaterial, LitlMatBinary* material, File const& file) noexcept
    {
        for (auto& shaderKvp : inputMaterial.shaders)
        {
            const auto& shader = shaderKvp.second;
            const auto type = toLowercase(shaderKvp.first);

            if (shader.resource.empty())
            {
                logWarning(".litlmat import of ", file.name(), ": shader '", type, "' missing required resource path ('resource'). Rejecting.");
                return false;
            }

            if (shader.entry.empty())
            {
                logWarning(".litlmat import of ", file.name(), ": shader '", type, "' missing required entry point ('entry'). Rejecting.");
                return false;
            }

            const auto mappedType = s_shaderTypeMap.find(type);

            if (mappedType == s_shaderTypeMap.end())
            {
                logWarning(".litlmat import of ", file.name(), ": shader '", type, "' invalid shader type '", type, "' specified. Rejecting.");
                return false;
            }

            switch (mappedType->second)
            {
            case LitlMatShaderStage::Vertex:
                material->setShaderVertex(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Fragment:
                material->setShaderFragment(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Geometry:
                material->setShaderGeometry(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::TessellationControl:
                material->setShaderTessellationControl(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::TessellationEvaluation:
                material->setShaderTessellationEvaluation(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Compute:
                material->setShaderCompute(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Mesh:
                material->setShaderMesh(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Task:
                material->setShaderTask(shader.resource, shader.entry);
                break;

            case LitlMatShaderStage::Unknown:
            default:
                logWarning(".litlmat import of ", file.name(), ": shader '", type, "' has unhandled  type of '", mappedType->first, "'. Rejecting.");
                return false;
            }
        }

        return true;
    }

    bool importRasterStateTable(ExpectedMaterialStructure const& inputMaterial, LitlMatBinary* material, File const& file) noexcept
    {
        const auto cullMode = toLowercase(inputMaterial.raster.cullMode);

        if (cullMode.empty() || cullMode == "back")
        {
            material->setRasterCullMode(LitlMatCullMode::Back);
        }
        else if (cullMode == "front")
        {
            material->setRasterCullMode(LitlMatCullMode::Front);
        }
        else if (cullMode == "none")
        {
            material->setRasterCullMode(LitlMatCullMode::None);
        }
        else if (cullMode == "both")
        {
            material->setRasterCullMode(LitlMatCullMode::Both);
        }
        else
        {
            logWarning(".litlmat import of ", file.name(), ": raster state 'cullMode' set to invalid value of '", cullMode, "'. Expected: 'back', 'front', 'none', 'both', or not provided. Rejecting.");
            return false;
        }

        const auto frontFace = toLowercase(inputMaterial.raster.frontFace);

        if (frontFace.empty() || frontFace == "clockwise" || frontFace == "cw")
        {
            material->setRasterWinding(true);
        }
        else if (frontFace == "counterclockwise" || frontFace == "counter-clockwise" || frontFace == "ccw")
        {
            material->setRasterWinding(false);
        }
        else
        {
            logWarning(".litlmat import of ", file.name(), ": raster state 'frontFace' set to invalid value of '", cullMode, "'. Expected: 'clockwise', 'cw', 'counterclockwise', 'counter-clockwise', 'ccw', or not provided. Rejecting.");
            return false;
        }

        return true;
    }

    bool importPropertiesTable(ExpectedMaterialStructure const& inputMaterial, LitlMatBinary* material, File const& file) noexcept
    {
        for (auto& propertyKvp : inputMaterial.properties)
        {
            const auto& property = propertyKvp.second;
            const auto type = toLowercase(property.type);

            if (type.empty())
            {
                logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' missing type specifier. Rejecting.");
                return false;
            }

            const auto mappedType = s_propertyTypeMap.find(type);

            if (mappedType == s_propertyTypeMap.end())
            {
                logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' invalid type '", type, "' specified. Rejecting.");
                return false;
            }

            switch (mappedType->second)
            {
            case LitlMatPropertyType::Bool:
                if (const auto* propertyValue = std::get_if<bool>(&propertyKvp.second.value))
                {
                    material->setBool(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Integer:
                if (const auto* propertyValue = std::get_if<int32_t>(&propertyKvp.second.value))
                {
                    material->setInt(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::UnsignedInteger:
                if (const auto* propertyValue = std::get_if<uint32_t>(&propertyKvp.second.value))
                {
                    material->setUint(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Float:
                if (const auto* propertyValue = std::get_if<float>(&propertyKvp.second.value))
                {
                    material->setFloat(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Double:
                if (const auto* propertyValue = std::get_if<float>(&propertyKvp.second.value))
                {
                    material->setDouble(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Vec2:
                if (const auto* propertyValue = std::get_if<std::vector<float>>(&propertyKvp.second.value))
                {
                    if (propertyValue->size() != 2)
                    {
                        logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' is labelled as a vec2 but has ", propertyValue->size(), " values. Expected 2. Rejecting.");
                        return false;
                    }

                    material->setVec2(propertyKvp.first, vec2{ propertyValue->at(0), propertyValue->at(1) });
                }
                break;

            case LitlMatPropertyType::Vec3:
                if (const auto* propertyValue = std::get_if<std::vector<float>>(&propertyKvp.second.value))
                {
                    if (propertyValue->size() != 3)
                    {
                        logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' is labelled as a vec3 but has ", propertyValue->size(), " values. Expected 3. Rejecting.");
                        return false;
                    }

                    material->setVec3(propertyKvp.first, vec3{ propertyValue->at(0), propertyValue->at(1), propertyValue->at(2) });
                }
                break;

            case LitlMatPropertyType::Vec4:
                if (const auto* propertyValue = std::get_if<std::vector<float>>(&propertyKvp.second.value))
                {
                    if (propertyValue->size() != 4)
                    {
                        logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' is labelled as a vec4 but has ", propertyValue->size(), " values. Expected 4. Rejecting.");
                        return false;
                    }

                    material->setVec4(propertyKvp.first, vec4{ propertyValue->at(0), propertyValue->at(1), propertyValue->at(2), propertyValue->at(3) });
                }
                break;

            case LitlMatPropertyType::Color:
                if (const auto* propertyValue = std::get_if<std::vector<float>>(&propertyKvp.second.value))
                {
                    if ((propertyValue->size() != 3) && (propertyValue->size() != 4))
                    {
                        logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' is labelled as a vec3 but has ", propertyValue->size(), " values. Expected 3 or 4. Rejecting.");
                        return false;
                    }

                    material->setColor(propertyKvp.first, color{ propertyValue->at(0), propertyValue->at(1), propertyValue->at(2), ((propertyValue->size() == 4) ? propertyValue->at(3) : 1.0f) });
                }
                break;

            case LitlMatPropertyType::Texture2D:
                if (const auto* propertyValue = std::get_if<std::string>(&propertyKvp.second.value))
                {
                    material->setTexture2D(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Texture3D:
                if (const auto* propertyValue = std::get_if<std::string>(&propertyKvp.second.value))
                {
                    material->setTexture3D(propertyKvp.first, *propertyValue);
                }
                break;

            case LitlMatPropertyType::Unknown:
            default:
                logWarning(".litlmat import of ", file.name(), ": property '", propertyKvp.first, "' has unhandled  type of '", mappedType->first, "'. Rejecting.");
                return false;
            }
        }

        return true;
    }

    bool importHintsTable(ExpectedMaterialStructure const& inputMaterial, LitlMatBinary* material, File const& file) noexcept
    {
        material->setHintFrequentUpdates(inputMaterial.hints.frequentUpdates);

        // ... add other hint types in the future ...

        return true;      // leave bool return to match other importX functions and for future compatibility.
    }

    Result LitlMatImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        const auto sourceBytesString = std::string_view{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size() };

        ExpectedMaterialStructure inputMaterial{};
        const auto readTomlResult = glz::read_toml(inputMaterial, sourceBytesString);

        if (readTomlResult != glz::error_code::none)
        {
            return Result::Error(ErrorType::ImporterFailed, readTomlResult.custom_error_message);
        }

        importedData.type = ImportedDataType::Material;
        importedData.material = std::make_unique<MaterialImportResult>();
        importedData.material->material = std::make_unique<LitlMatBinary>();
        auto* material = importedData.material->material.get();

        if (!inputMaterial.name.empty())
        {
            material->setName(inputMaterial.name);
        }
        else
        {
            material->setName(file.name());
        }

        if (!importShadersTable(inputMaterial, material, file))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [properties] table.", file.name()));
        }

        if (!importRasterStateTable(inputMaterial, material, file))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [raster] table.", file.name()));
        }

        if (!importPropertiesTable(inputMaterial, material, file))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [properties] table.", file.name()));
        }

        if (!importHintsTable(inputMaterial, material, file))
        {
            return Result::Error(ErrorType::ImporterFailed, std::format(".litlmat import of {}: Error encountered importing [hints] table.", file.name()));
        }

        return Result::Success();
    }
}