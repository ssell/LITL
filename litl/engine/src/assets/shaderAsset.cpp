#include "litl-core/logging/logging.hpp"
#include "litl-engine/assets/shaderAsset.hpp"
#include "litl-engine/objects/objectPool.hpp"
#include "litl-import/importService.hpp"
#include "litl-import/shader/intermediate/litlshader.hpp"

namespace litl
{
    bool ShaderAsset::fetchAssetObject(Asset* asset, ObjectPool& objectPool) noexcept
    {
        ShaderAsset* shaderAsset = static_cast<ShaderAsset*>(asset);
        shaderAsset->shader = objectPool.getShader(shaderAsset->handle);

        return (shaderAsset->shader != nullptr);
    }

    namespace
    {
        [[nodiscard]] bool decodeLitlShaderBytes(ShaderAsset* shaderAsset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
        {
            import::LitlShader litlshader;
            BinaryBlockFile::ErrorCode litlshaderError = BinaryBlockFile::ErrorCode::None;

            if (!import::LitlShader::parse(bytes, litlshader, litlshaderError))
            {
                logError("Failed to parse shader asset with error code ", static_cast<uint32_t>(litlshaderError));
                error = AssetErrorCode::ParseFailed;
                return false;
            }

            shaderAsset->shaderIntermediateData = std::make_shared<import::ShaderIntermediateData>();

            if (!litlshader.deserialize(*shaderAsset->shaderIntermediateData, litlshaderError))
            {
                logError("Failed to decode shader asset with error code ", static_cast<uint32_t>(litlshaderError));
                error = AssetErrorCode::DeserializationFailed;
            }

            return true;
        }

        [[nodiscard]] bool decodeNonLitShaderBytes(ShaderAsset* shaderAsset, std::span<std::byte const> otherBytes, AssetErrorCode& error) noexcept
        {
            const auto extension = shaderAsset->file.extension();

            import::ImportService importer{};
            import::ImportedData importedData{};

            const auto importResult = importer.import(shaderAsset->file, otherBytes, importedData, true);

            if (importResult.success)
            {
                if (importedData.type == import::ImportedDataType::Shader)
                {
                    shaderAsset->shaderIntermediateData = importedData.shader->intermediateShader;
                    return true;
                }
                else
                {
                    logError("Import of shader bytes from third-party asset failed due to detected import format was not shader but instead format type ", static_cast<uint32_t>(importedData.type));
                    error = AssetErrorCode::ExternalFormatImportFailed;
                    return false;
                }
            }
            else
            {
                logError("Failed to import bytes of shader from third-party asset with message '", importResult.message, "' and error code ", static_cast<uint32_t>(importResult.error));
                error = AssetErrorCode::ExternalFormatImportFailed;
                return false;
            }

            return true;
        }
    }

    bool ShaderAsset::decodeBytes(Asset* asset, std::span<std::byte const> bytes, AssetErrorCode& error) noexcept
    {
        if (bytes.empty())
        {
            error = AssetErrorCode::DecodeBytesEmpty;
            return false;
        }

        ShaderAsset* shaderAsset = static_cast<ShaderAsset*>(asset);

        if (shaderAsset->file.extension() == ".litlshader")
        {
            // Already a .litlshader, so we can just decode straight to our LitlShader struct.
            return decodeLitlShaderBytes(shaderAsset, bytes, error);
        }
        else
        {
            logWarning("Decoding shader asset with key '", asset->key, "' directly from external format. It is recommended to first convert the mesh to the internal .litlshader format to improve loading performance.");
            return decodeNonLitShaderBytes(shaderAsset, bytes, error);
        }

        return true;
    }

    bool ShaderAsset::processOnWorker(Asset* asset, AssetErrorCode& error) noexcept
    {
        // ... no action ...
        return true;
    }

    bool ShaderAsset::processOnMain(Asset* asset, ObjectPool& objectPool, AssetErrorCode& error) noexcept
    {
        ShaderAsset* shaderAsset = static_cast<ShaderAsset*>(asset);

        if (shaderAsset->shader == nullptr)
        {
            logError("Processing ShaderAsset '", shaderAsset->key, "' failed as shader object is null.");
            return false;
        }

        if (shaderAsset->shaderIntermediateData == nullptr)
        {
            logError("Processing ShaderAsset '", shaderAsset->key, "' failed as intermediate data is null.");
            return false;
        }

        const bool success = shaderAsset->shader->setData(*shaderAsset->shaderIntermediateData);

        if (!success)
        {
            logError("Failed to create renderer ShaderModule for ShaderAsset '", shaderAsset->key, "'");
        }

        shaderAsset->shaderIntermediateData = nullptr;

        return success;
    }
}