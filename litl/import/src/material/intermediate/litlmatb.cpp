#include <array>
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
            uint32_t length{ 0u };
            std::span<std::byte const> value;
        };

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

                switch (property.type)
                {
                case LitlMatPropertyType::Bool:
                    binaryPropertyRecord.length = 1u;
                    binaryPropertyRecord.value = as_byte_span(std::get<uint8_t>(property.value));
                    break;

                case LitlMatPropertyType::Integer:
                    binaryPropertyRecord.length = 4u;
                    binaryPropertyRecord.value = as_byte_span(std::get<int32_t>(property.value));
                    break;

                case LitlMatPropertyType::UnsignedInteger:
                    binaryPropertyRecord.length = 4u;
                    binaryPropertyRecord.value = as_byte_span(std::get<uint32_t>(property.value));
                    break;

                case LitlMatPropertyType::Float:
                    binaryPropertyRecord.length = 4u;
                    binaryPropertyRecord.value = as_byte_span(std::get<float>(property.value));
                    break;

                case LitlMatPropertyType::Double:
                    binaryPropertyRecord.length = 8u;
                    binaryPropertyRecord.value = as_byte_span(std::get<double>(property.value));
                    break;

                case LitlMatPropertyType::Vec2:
                    binaryPropertyRecord.length = 8u;
                    binaryPropertyRecord.value = as_byte_span(std::get<vec2>(property.value));
                    break;

                case LitlMatPropertyType::Vec3:
                    binaryPropertyRecord.length = 12u;
                    binaryPropertyRecord.value = as_byte_span(std::get<vec3>(property.value));
                    break;

                case LitlMatPropertyType::Vec4:
                    binaryPropertyRecord.length = 16u;
                    binaryPropertyRecord.value = as_byte_span(std::get<vec4>(property.value));
                    break;

                case LitlMatPropertyType::Color:
                    binaryPropertyRecord.length = 16u;
                    binaryPropertyRecord.value = as_byte_span(std::get<color>(property.value));
                    break;

                case LitlMatPropertyType::Texture2D:
                case LitlMatPropertyType::Texture3D:
                    binaryPropertyRecord.length = sizeof(BinaryBlockFile::StringRef);
                    binaryPropertyRecord.value = as_byte_span(std::get<std::string>(property.value));
                    break;
                }

                propertyRecords.push_back(binaryPropertyRecord);
            }
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
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Shaders, sizeof(LitlMatShaderRecord), as_byte_span(shaderRecords) });

        std::vector<LitlMatBinaryPropertyRecord> propertyRecords;
        compilePropertyRecords(material, propertyRecords, stringMap);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Properties, sizeof(LitlMatPropertyRecord), as_byte_span(propertyRecords) });

        LitlMatBinaryRasterSettings rasterSettings{};
        compileRasterSettings(material, rasterSettings);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Raster, sizeof(LitlMatRasterSettings), as_byte_span(rasterSettings) });

        LitlMatBinaryHintSettings hintSettings{};
        compileHintSettings(material, hintSettings);
        blockDataTable.push_back(BlockDataDescriptor{ &litlMatBinary.descriptors[blockDataTable.size()], BlockIds::Hints, sizeof(LitlMatHintSettings), as_byte_span(hintSettings) });

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

        for (size_t i = 0ull; i < blockDataTable.size(); ++i)
        {
            std::memcpy(data.data() + litlMatBinary.header.descriptorsOffset + (sizeof(BlockDescriptor) * i), blockDataTable[i].descriptor, sizeof(BlockDescriptor));
        }

        for (auto& blockData : blockDataTable)
        {
            if (blockData.data.size() > 0)
            {
                std::memcpy(data.data() + blockData.descriptor->blockOffset, blockData.data.data(), blockData.data.size());
            }
        }

        litlMatBinary.header.contentHash = calculateContentHash(std::span<std::byte const>(data), litlMatBinary.header);

        std::memcpy(data.data(), &litlMatBinary.header, sizeof(Header));

        return true;
    }

    bool LitlMatBinary::deserialize(MaterialIntermediateData& material, ErrorCode& error) const noexcept
    {
        // ... todo ...
        return true;
    }
}