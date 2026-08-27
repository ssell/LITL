#include <expected>
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

    Result LitlMatImporter::import(File const& file, std::span<std::byte const> sourceBytes, ImportedData& importedData) noexcept
    {
        const auto sourceBytesString = std::string_view{ reinterpret_cast<char const*>(sourceBytes.data()), sourceBytes.size() };

        ExpectedMaterialStructure inputMaterial{};
        const auto readTomlResult = glz::read_toml(inputMaterial, sourceBytesString);

        if (!readTomlResult)
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

        // ... todo set shaders ...
        // ... todo set raster state ...

        for (auto& propertyKvp : inputMaterial.properties)
        {
            auto& property = propertyKvp.second;
            auto type = toLowercase(property.type);

            if (type.empty())
            {
                logWarning(".litlmat import: property '", propertyKvp.first, "' missing type specifier. Skipping.");
                continue;
            }

            auto mappedType = s_propertyTypeMap.find(type);

            if (mappedType == s_propertyTypeMap.end())
            {
                logWarning(".litlmat import: property '", propertyKvp.first, "' invalid type '", type, "' specified. Skipping.");
                continue;
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
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Vec3:
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Vec4:
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Color:
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Texture2D:
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Texture3D:
                    // ... todo ...
                    break;

                case LitlMatPropertyType::Unknown:
                default:
                    logWarning(".litlmat import: property '", propertyKvp.first, "' has unhandled  type of '", mappedType->first, "'. Skipping.");
                    continue;
            }
        }

        // ... todo set hints ...

        return Result::Success();
    }
}