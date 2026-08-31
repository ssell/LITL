#include <array>
#include <bit>
#include <cstring>

#include "litl-core/logging/logging.hpp"
#include "litl-core/containers/common.hpp"
#include "litl-import/material/intermediate/litlmatb.hpp"

namespace litl::import
{
    namespace
    {
        struct LitlMatBinaryShaderRecord
        {
            LitlMatShaderStage stage{ LitlMatShaderStage::Unknown };
            BinaryBlockFile::StringRef resource{};
            BinaryBlockFile::StringRef entry{};
            uint64_t padding{ 0ull };
        };

        static_assert(sizeof(LitlMatBinaryShaderRecord) == 48);
        static_assert(std::is_trivially_copyable_v<LitlMatBinaryShaderRecord>);

        struct LitlMatBinaryPropertyRecord
        {
            BinaryBlockFile::StringRef name;
            LitlMatPropertyType type;
            std::array<std::byte, 24> value;        // size of the largest property (mat4 or string ref) sized up to nearest multiple of 16. wasteful for small property types, but theres only so many properties a material will have.
        };

        static_assert(sizeof(LitlMatBinaryPropertyRecord) == 48);
        static_assert(std::is_trivially_copyable_v<LitlMatBinaryPropertyRecord>);

        struct LitlMatBinaryRasterSettings
        {
            LitlMatCullMode cullMode{ LitlMatCullMode::Back };
            bool clockwise{ true };
            std::array<uint8_t, 3> padding{};
        };

        static_assert(sizeof(LitlMatBinaryRasterSettings) == 8);
        static_assert(std::is_trivially_copyable_v<LitlMatBinaryRasterSettings>);

        struct LitlMatBinaryHintSettings
        {
            bool frequentUpdates{ false };
            std::array<uint8_t, 3> padding{};
        };

        static_assert(sizeof(LitlMatBinaryHintSettings) == 4);
        static_assert(std::is_trivially_copyable_v<LitlMatBinaryHintSettings>);

        // -------------------------------------------------------------------------------------
        // Serialization
        // -------------------------------------------------------------------------------------

        void compileShaderRecords(MaterialIntermediateData const& material, std::vector<LitlMatBinaryShaderRecord>& shaderRecords, BinaryBlockFile::StringMap& stringMap) noexcept
        {
            // Note MaterialIntermediateData guarantees 1 shader entry per stage. If that ever changes, then dedupe checking will need to be done.
            for (auto& shader : material.getShaders())
            {
                if ((shader.stage == LitlMatShaderStage::Unknown) || shader.resource.empty() || shader.entry.empty())
                {
                    continue;
                }

                shaderRecords.push_back(LitlMatBinaryShaderRecord{
                    .stage = shader.stage,
                    .resource = BinaryBlockFile::serializeString(shader.resource, stringMap),
                    .entry = BinaryBlockFile::serializeString(shader.entry, stringMap)
                });
            }
        }

        void compilePropertyRecords(MaterialIntermediateData const& material, std::vector<LitlMatBinaryPropertyRecord>& propertyRecords, BinaryBlockFile::StringMap& stringMap) noexcept
        {
            for (auto& property : material.getProperties())
            {
                auto binaryPropertyRecord = LitlMatBinaryPropertyRecord{
                    .name = BinaryBlockFile::serializeString(property.name, stringMap),
                    .type = property.type
                };

                binaryPropertyRecord.value.fill(std::byte{ 0 });

                switch (property.type)
                {
                case LitlMatPropertyType::Bool:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<uint8_t>(property.value), sizeof(uint8_t));
                    break;

                case LitlMatPropertyType::Integer:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<int32_t>(property.value), sizeof(int32_t));
                    break;

                case LitlMatPropertyType::UnsignedInteger:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<uint32_t>(property.value), sizeof(uint32_t));
                    break;

                case LitlMatPropertyType::Float:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<float>(property.value), sizeof(float));
                    break;

                case LitlMatPropertyType::Double:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<double>(property.value), sizeof(double));
                    break;

                case LitlMatPropertyType::Vec2:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<vec2>(property.value), sizeof(vec2));
                    break;

                case LitlMatPropertyType::Vec3:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<vec3>(property.value), sizeof(vec3));
                    break;

                case LitlMatPropertyType::Vec4:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<vec4>(property.value), sizeof(vec4));
                    break;

                case LitlMatPropertyType::Color:
                    std::memcpy(binaryPropertyRecord.value.data(), &std::get<color>(property.value), sizeof(color));
                    break;

                case LitlMatPropertyType::Texture2D:
                case LitlMatPropertyType::Texture3D:
                    {
                        auto stringRef = BinaryBlockFile::serializeString(std::get<std::string>(property.value), stringMap);
                        std::memcpy(binaryPropertyRecord.value.data(), &stringRef, sizeof(BinaryBlockFile::StringRef));
                    }
                    break;

                case LitlMatPropertyType::Unknown:
                    continue;
                }

                propertyRecords.push_back(binaryPropertyRecord);
            }
        }

        void serializePropertiesBlock(void* dest, BinaryBlockFile::BlockDataDescriptor const& blockData, std::vector<LitlMatBinaryPropertyRecord>& propertyRecords) noexcept
        {
            size_t propertiesTotalSize = 0;

            for (auto& property : propertyRecords)
            {
                propertiesTotalSize += sizeof(LitlMatBinaryPropertyRecord);
            }

            std::vector<std::byte> propertiesBlock;
        }

        void compileRasterSettings(MaterialIntermediateData const& material, LitlMatBinaryRasterSettings& rasterSettings) noexcept
        {
            auto const& settings = material.getRasterSettings();

            rasterSettings.cullMode = settings.cullMode;
            rasterSettings.clockwise = settings.clockwise;
        }

        void compileHintSettings(MaterialIntermediateData const& material, LitlMatBinaryHintSettings& hintSettings) noexcept
        {
            auto const& settings = material.getHintSettings();
            hintSettings.frequentUpdates = settings.frequentUpdates;
        }
    }

    bool LitlMatBinary::serialize(MaterialIntermediateData const& material, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        error = ErrorCode::None;

        LitlMatBinary litlMatBinary{};
        StringMap stringMap{};

        std::vector<BlockDataDescriptor> blockDataTable; blockDataTable.reserve(MaxBlocks);
        litlMatBinary.addDefaultBlockDescriptors(blockDataTable);

        std::vector<LitlMatBinaryShaderRecord> shaderRecords;
        compileShaderRecords(material, shaderRecords, stringMap);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Shaders, sizeof(LitlMatBinaryShaderRecord), as_byte_span(shaderRecords) });

        std::vector<LitlMatBinaryPropertyRecord> propertyRecords;
        compilePropertyRecords(material, propertyRecords, stringMap);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Properties, sizeof(LitlMatBinaryPropertyRecord), as_byte_span(propertyRecords) });

        LitlMatBinaryRasterSettings rasterSettings{};
        compileRasterSettings(material, rasterSettings);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Raster, sizeof(LitlMatBinaryRasterSettings), as_byte_span(rasterSettings) });

        LitlMatBinaryHintSettings hintSettings{};
        compileHintSettings(material, hintSettings);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Hints, sizeof(LitlMatBinaryHintSettings), as_byte_span(hintSettings) });

        for (auto& blockData : blockDataTable)
        {
            if (blockData.elementSize == 0ull)
            {
                error = ErrorCode::ElementSizeOfZero;
                return false;
            }

            if (blockData.data.size() % blockData.elementSize != 0)
            {
                error = ErrorCode::ElementBlockIsNotWhole;
                return false;
            }
        }

        // ---------------------------------------------------------------------------------
        // Populate Header (most of it)

        litlMatBinary.header.magic = Identity.magic;
        litlMatBinary.header.versionMajor = Identity.versionMajor;
        litlMatBinary.header.versionMinor = Identity.versionMinor;
        litlMatBinary.header.contentHash = 0ull;         // calculated further on
        litlMatBinary.header.totalBytes = 0u;            // calculated further on
        litlMatBinary.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlMatBinary.header.descriptorsOffset = sizeof(Header);
        litlMatBinary.header.blocksOffset = litlMatBinary.header.descriptorsOffset + (sizeof(BlockDescriptor) * litlMatBinary.header.blockCount);
        litlMatBinary.header.flags = 0u;

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        litlMatBinary.serializeDefaultBlocks(blockDataTable, stringMap);

        uint64_t runningOffset = litlMatBinary.header.blocksOffset;

        for (uint32_t i = 0; i < litlMatBinary.header.blockCount; ++i)
        {
            auto& blockData = blockDataTable[i];
            serializeBlock(blockData, runningOffset);
        }

        litlMatBinary.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        data.resize(litlMatBinary.header.totalBytes);
        std::fill(data.begin(), data.end(), std::byte(0));

        // Copy the descriptors
        for (size_t i = 0ull; i < blockDataTable.size(); ++i)
        {
            std::memcpy(data.data() + litlMatBinary.header.descriptorsOffset + (sizeof(BlockDescriptor) * i), blockDataTable[i].descriptor, sizeof(BlockDescriptor));
        }

        // Copy the data
        for (auto& blockData : blockDataTable)
        {
            if (blockData.data.size() > 0)
            {
                std::memcpy(data.data() + blockData.descriptor->blockOffset, blockData.data.data(), blockData.data.size());
            }
        }

        // Calculate hash and then copy the header
        litlMatBinary.header.contentHash = calculateContentHash(std::span<std::byte const>(data), litlMatBinary.header);
        std::memcpy(data.data(), &litlMatBinary.header, sizeof(Header));

        return true;
    }

    // -------------------------------------------------------------------------------------
    // Deserialization
    // -------------------------------------------------------------------------------------

    namespace
    {
        [[nodiscard]] bool deserializeShadersBlock(MaterialIntermediateData& material, BinaryBlockFile::Block& shadersBlock, std::span<char const> strings, BinaryBlockFile::ErrorCode& error) noexcept
        {
            auto shaderRecords = shadersBlock.as<LitlMatBinaryShaderRecord>(error);

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return false;
            }

            if (!shaderRecords.has_value())
            {
                // Not a deserialization error, though it may be a validation error. But that is up to the Material to decide.
                return true;
            }

            for (auto& shaderRecord : shaderRecords.value())
            {
                const auto resource = BinaryBlockFile::deserializeString(strings, shaderRecord.resource, error);

                if (error != BinaryBlockFile::ErrorCode::None)
                {
                    return false;
                }

                const auto entry = BinaryBlockFile::deserializeString(strings, shaderRecord.entry, error);

                if (error != BinaryBlockFile::ErrorCode::None)
                {
                    return false;
                }

                if (!material.setShader(shaderRecord.stage, std::string(resource), std::string(entry)))
                {
                    error = BinaryBlockFile::ErrorCode::MaterialPropertyDeserializationFailed;
                    return false;
                }
            }

            return true;
        }

        template<typename T> requires std::is_trivially_copyable_v<T>
        void deserializeTrivialPropertyType(MaterialIntermediateData& material, std::span<char const> strings, LitlMatBinaryPropertyRecord const& propertyRecord, BinaryBlockFile::ErrorCode& error) noexcept
        {
            const std::string propertyName = std::string(BinaryBlockFile::deserializeString(strings, propertyRecord.name, error));

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return;
            }

            const std::optional<T> value = from_generic_byte_span<T>(propertyRecord.value, 0);

            if (!value.has_value() || !material.addProperty(propertyName, propertyRecord.type, value.value()))
            {
                error = BinaryBlockFile::ErrorCode::MaterialPropertyDeserializationFailed;
            }
        }

        void deserializeStringPropertyType(MaterialIntermediateData& material, std::span<char const> strings, LitlMatBinaryPropertyRecord const& propertyRecord, BinaryBlockFile::ErrorCode& error) noexcept
        {
            const std::string propertyName = std::string(BinaryBlockFile::deserializeString(strings, propertyRecord.name, error));

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return;
            }

            const std::optional<BinaryBlockFile::StringRef> valueRef = from_generic_byte_span<BinaryBlockFile::StringRef>(propertyRecord.value, 0);

            if (!valueRef.has_value())
            {
                error = BinaryBlockFile::ErrorCode::MaterialPropertyDeserializationFailed;
                return;
            }

            const std::string value = std::string(BinaryBlockFile::deserializeString(strings, valueRef.value(), error));

            if (value.empty() || !material.addProperty(propertyName, propertyRecord.type, value))
            {
                error = BinaryBlockFile::ErrorCode::MaterialPropertyDeserializationFailed;
            }
        }

        [[nodiscard]] bool deserializePropertiesBlock(MaterialIntermediateData& material, BinaryBlockFile::Block& propertiesBlock, std::span<char const> strings, BinaryBlockFile::ErrorCode& error) noexcept
        {
            auto propertyRecords = propertiesBlock.as<LitlMatBinaryPropertyRecord>(error);

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return false;
            }

            for (auto& propertyRecord : (*propertyRecords))
            {
                const auto propertyName = std::string(BinaryBlockFile::deserializeString(strings, propertyRecord.name, error));

                if (error != BinaryBlockFile::ErrorCode::None)
                {
                    return false;
                }

                switch (propertyRecord.type)
                {
                case LitlMatPropertyType::Bool:
                    deserializeTrivialPropertyType<uint8_t>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Integer:
                    deserializeTrivialPropertyType<int32_t>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::UnsignedInteger:
                    deserializeTrivialPropertyType<uint32_t>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Float:
                    deserializeTrivialPropertyType<float>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Double:
                    deserializeTrivialPropertyType<double>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Vec2:
                    deserializeTrivialPropertyType<vec2>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Vec3:
                    deserializeTrivialPropertyType<vec3>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Vec4:
                    deserializeTrivialPropertyType<vec4>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Color:
                    deserializeTrivialPropertyType<color>(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Texture2D:
                case LitlMatPropertyType::Texture3D:
                    deserializeStringPropertyType(material, strings, propertyRecord, error);
                    break;

                case LitlMatPropertyType::Unknown:
                default:
                    error = BinaryBlockFile::ErrorCode::UnknownMaterialPropertyType;
                    break;
                }

                if (error != BinaryBlockFile::ErrorCode::None)
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] bool deserializeRasterSettingsBlock(MaterialIntermediateData& material, BinaryBlockFile::Block& rasterSettingsBlock, BinaryBlockFile::ErrorCode& error) noexcept
        {
            auto rasterSettingsElements = rasterSettingsBlock.as<LitlMatBinaryRasterSettings>(error);

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return false;
            }

            if (rasterSettingsElements->empty())
            {
                // None specified, will use the defaults.
                return true;
            }

            auto& rasterSettings = (*rasterSettingsElements)[0];

            material.setRasterCullMode(rasterSettings.cullMode);
            material.setRasterWinding(rasterSettings.clockwise);
            // ... add others as needed ...

            return true;
        }

        [[nodiscard]] bool deserializeHintSettingsBlock(MaterialIntermediateData& material, BinaryBlockFile::Block& hintSettingsBlock, BinaryBlockFile::ErrorCode& error) noexcept
        {
            auto hintSettingsElements = hintSettingsBlock.as<LitlMatBinaryHintSettings>(error);

            if (error != BinaryBlockFile::ErrorCode::None)
            {
                return false;
            }

            if (hintSettingsElements->empty())
            {
                // None specified, will use the defaults.
                return true;
            }

            auto& hintSettings = (*hintSettingsElements)[0];

            material.setHintFrequentUpdates(hintSettings.frequentUpdates);
            // ... add others as needed ...

            return true;
        }
    }

    bool LitlMatBinary::deserialize(MaterialIntermediateData& material, ErrorCode& error) const noexcept
    {
        auto stringsBlock = find(DefaultBlocks::Strings);
        auto shadersBlock = find(BlockIds::Shaders);
        auto propertiesBlock = find(BlockIds::Properties);
        auto rasterSettingsBlock = find(BlockIds::Raster);
        auto hintSettingsBlock = find(BlockIds::Hints);

        if (!stringsBlock.has_value())
        {
            error = ErrorCode::MissingStringsBlock;
            return false;
        }

        if (!shadersBlock.has_value())
        {
            error = ErrorCode::MissingShadersBlock;
            return false;
        }

        if (!propertiesBlock.has_value())
        {
            error = ErrorCode::MissingPropertiesBlock;
            return false;
        }

        if (!rasterSettingsBlock.has_value())
        {
            error = ErrorCode::MissingRasterSettingsBlock;
            return false;
        }

        if (!hintSettingsBlock.has_value())
        {
            error = ErrorCode::MissingHintSettingsBlock;
            return false;
        }

        auto strings = stringsBlock->as<char const>(error);

        if (error != ErrorCode::None)
        {
            return false;
        }

        return deserializeShadersBlock(material, (*shadersBlock), (*strings), error) &&
               deserializePropertiesBlock(material, (*propertiesBlock), (*strings), error) &&
               deserializeRasterSettingsBlock(material, (*rasterSettingsBlock), error) &&
               deserializeHintSettingsBlock(material, (*hintSettingsBlock), error);
    }
}