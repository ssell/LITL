#include <array>
#include <type_traits>

#include "litl-core/containers/common.hpp"
#include "litl-import/shader/intermediate/litlshader.hpp"

namespace litl::import
{
    namespace
    {
        /// <summary>
        /// A sequential grouping of Brecords.
        /// Composed of the number of properties in the group and the local offset into block of the first property.
        /// </summary>
        struct BinaryRecordGrouping
        {
            uint64_t count{ 0ull };
            uint64_t firstOffset{ 0ull };
        };

        static_assert(sizeof(BinaryRecordGrouping) == 16);
        static_assert(sizeof(BinaryRecordGrouping) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryRecordGrouping>);

        /// <summary>
        /// Maps to: ShaderVariable
        /// </summary>
        struct BinaryShaderVariable
        {
            uint32_t scalarType{ 0u };
            uint32_t flag{ 0u };
            uint32_t scalarSize{ 0u };
            uint32_t componentCount{ 0u };
            uint32_t matrixStride{ 0u };
            uint32_t arrayStride{ 0u };
            uint32_t arrayDimensionsCount{ 0u };
            uint32_t arrayDimensions[ShaderVariable::MaxArrayDimensions] = {};
            uint32_t padding{ 0u };
        };

        static_assert(ShaderVariable::MaxArrayDimensions == 4);
        static_assert(sizeof(BinaryShaderVariable) == 48);
        static_assert(sizeof(BinaryShaderVariable) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryShaderVariable>);

        /// <summary>
        /// Maps to: ResourceProperty
        /// The 'RPRP' block stores N number of these. They are stored sequentially in that sibling properties are next to each other. 
        /// For structures that have a variable number of resource properties, such as ResourceBinding, PushConstantRange, etc. 
        /// they store the number of resource properties they have and the offset of the first one in the RPRP block.
        /// </summary>
        struct BinaryResourceProperty
        {
            BinaryBlockFile::StringRef name;
            uint64_t hashedName{ 0ull };
            BinaryShaderVariable variable{ 0u };
            uint32_t offset{ 0u };
            uint32_t size{ 0u };
            uint32_t sizePadded{ 0u };
            std::array<uint32_t, 3> padding{};
        };

        static_assert(sizeof(BinaryResourceProperty) == 96);
        static_assert(sizeof(BinaryResourceProperty) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryResourceProperty>);

        /// <summary>
        /// Maps to: ResourceBinding
        /// </summary>
        struct BinaryResourceBinding
        {
            BinaryBlockFile::StringRef name;
            uint64_t hashedName{ 0ull };
            uint32_t type{ 0u };
            uint32_t set{ 0u };
            uint32_t binding{ 0u };
            uint32_t arraySize{ 0u };
            uint32_t size{ 0u };
            BinaryRecordGrouping resourceProperties{};
        };

        static_assert(sizeof(BinaryResourceBinding) == 64);
        static_assert(sizeof(BinaryResourceBinding) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryResourceBinding>);

        /// <summary>
        /// Maps to: PushConstantReferenceProperty
        /// The 'RFLC' block stores N number of these. They are stored sequentially in that sibling properties are next to each other. 
        /// </summary>
        struct BinaryPushConstantReferenceProperty
        {
            BinaryBlockFile::StringRef name;
            uint64_t hashedName{ 0ull };
            uint32_t offset{ 0u };
            uint32_t size{ 0u };
            uint32_t stride{ 0u };
            uint32_t padding0{ 0u };
            BinaryRecordGrouping resourceProperties{};
            uint64_t padding1{ 0ull };
        };

        static_assert(sizeof(BinaryPushConstantReferenceProperty) == 64);
        static_assert(sizeof(BinaryPushConstantReferenceProperty) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryPushConstantReferenceProperty>);

        /// <summary>
        /// Maps to: PushConstantRange
        /// </summary>
        struct BinaryPushConstantRange
        {
            uint32_t offset{ 0u };
            uint32_t size{ 0u };
            BinaryRecordGrouping resourceProperties{};
            BinaryRecordGrouping referenceProperties{};
            uint64_t padding{ 0ull };
        };

        static_assert(sizeof(BinaryPushConstantRange) == 48);
        static_assert(sizeof(BinaryPushConstantRange) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryPushConstantRange>);

        /// <summary>
        /// Maps to: ShaderInputOutputVariable
        /// </summary>
        struct BinaryShaderInputOutputVariable
        {
            BinaryBlockFile::StringRef name;
            BinaryShaderVariable variable{ 0u };
            uint32_t location{ 0u };
            uint32_t componentCount{ 0u };
            std::array<uint32_t, 6> padding;
        };

        static_assert(sizeof(BinaryShaderInputOutputVariable) == 96);
        static_assert(sizeof(BinaryShaderInputOutputVariable) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryShaderInputOutputVariable>);

        /// <summary>
        /// Maps to: SpecializationConstant
        /// </summary>
        struct BinarySpecializationConstant
        {
            BinaryBlockFile::StringRef name;
            uint32_t id{ 0u };
            uint32_t variable{ 0u };
            uint64_t padding{ 0ull };
        };

        static_assert(sizeof(BinarySpecializationConstant) == 32);
        static_assert(sizeof(BinarySpecializationConstant) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinarySpecializationConstant>);

        /// <summary>
        /// Maps to: ComputeInfo
        /// </summary>
        struct BinaryComputeInfo
        {
            uint32_t localSizeX{ 0u };
            uint32_t localSizeY{ 0u };
            uint32_t localSizeZ{ 0u };
            uint32_t padding{ 0u };
        };

        static_assert(sizeof(BinaryComputeInfo) == 16);
        static_assert(sizeof(BinaryComputeInfo) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryComputeInfo>);

        /// <summary>
        /// Maps to: PushConstantRange
        /// </summary>
        struct BinaryEntryPointReflection
        {
            BinaryBlockFile::StringRef entryPoint;
            uint32_t stage{ 0u };
            uint32_t padding0{ 0u };
            BinaryRecordGrouping resources{};
            BinaryRecordGrouping pushConstants{};
            BinaryRecordGrouping vertexInputs{};
            BinaryRecordGrouping fragmentOutputs{};
            BinaryComputeInfo computeInfo{};
            uint64_t padding1{ 0u };
        };

        static_assert(sizeof(BinaryEntryPointReflection) == 112);
        static_assert(sizeof(BinaryEntryPointReflection) % 16 == 0);
        static_assert(std::is_trivially_copyable_v<BinaryEntryPointReflection>);

        struct LitlShaderBlocksData
        {
            BinaryBlockFile::StringMap stringMap{};

            std::vector<BinaryResourceProperty> resourceProperties;                             // 'RESP' (RESource Properties)
            std::vector<BinaryShaderInputOutputVariable> shaderInputOutput;                     // 'VFIO' (Vertex Fragment Input/Output)
            std::vector<BinaryPushConstantReferenceProperty> pushConstantReferenceProperties;   // 'PURP' (PUsh constant Reference Properties)
            std::vector<BinaryPushConstantRange> pushConstants;                                 // 'PUSH' (PUSH constants)
            std::vector<BinaryResourceBinding> resourceBindings;                                // 'RESB' (RESource Bindings)
            std::vector<BinaryEntryPointReflection> entryPoints;                                // 'ENTR' (ENTRy points)

            uint64_t runningResourcePropertyOffset = 0ull;
            uint64_t runningResourceBindingOffset = 0ull;
            uint64_t runningPushConstantsOffset = 0ull;
            uint64_t runningPushConstantReferencePropertyOffset = 0ull;
            uint64_t runningVertexFragmentInputOutputOffset = 0ull;
        };

        [[nodiscard]] BinaryRecordGrouping nextBinaryRecordGrouping(size_t elementSize, size_t elementCount, uint64_t& runningOffset) noexcept
        {
            const BinaryRecordGrouping grouping{ .count = elementCount, .firstOffset = runningOffset };
            runningOffset += elementSize * elementCount;
            return grouping;
        }

        [[nodiscard]] BinaryShaderVariable serializeShaderVariable(ShaderVariable const& shaderVariable) noexcept
        {
            auto binaryShaderVariable = BinaryShaderVariable{
                .scalarType = static_cast<uint32_t>(shaderVariable.scalarType),
                .scalarSize = shaderVariable.scalarSize,
                .componentCount = shaderVariable.componentCount,
                .matrixStride = shaderVariable.matrixStride,
                .arrayStride = shaderVariable.arrayStride,
                .arrayDimensionsCount = shaderVariable.arrayDimensionsCount
            };

            for (auto i = 0; i < ShaderVariable::MaxArrayDimensions; ++i)
            {
                binaryShaderVariable.arrayDimensions[i] = shaderVariable.arrayDimensions[i];
            }

            return binaryShaderVariable;
        }

        [[nodiscard]] BinaryResourceProperty serializeResourceProperty(ResourceProperty const& resourceProperty, LitlShaderBlocksData& blocksData) noexcept
        {
            return BinaryResourceProperty {
                .name = BinaryBlockFile::serializeString(resourceProperty.name, blocksData.stringMap),
                .hashedName = resourceProperty.hashedName.value,
                .variable = serializeShaderVariable(resourceProperty.variable),
                .offset = resourceProperty.offset,
                .size = resourceProperty.size,
                .sizePadded = resourceProperty.sizePadded
            };
        }

        [[nodiscard]] BinaryResourceBinding serializeResourceBinding(ResourceBinding const& resourceBinding, LitlShaderBlocksData& blocksData) noexcept
        {
            BinaryResourceBinding binaryResourceBinding{
                        .name = BinaryBlockFile::serializeString(resourceBinding.name, blocksData.stringMap),
                        .hashedName = resourceBinding.hashedName.value,
                        .type = static_cast<uint32_t>(resourceBinding.type),
                        .set = resourceBinding.set,
                        .binding = resourceBinding.binding,
                        .arraySize = resourceBinding.arraySize,
                        .size = resourceBinding.sizeBytes
            };

            if (!resourceBinding.properties.empty())
            {
                binaryResourceBinding.resourceProperties = nextBinaryRecordGrouping(sizeof(BinaryResourceProperty), resourceBinding.properties.size(), blocksData.runningResourcePropertyOffset);

                for (auto& resourceProperty : resourceBinding.properties)
                {
                    blocksData.resourceProperties.push_back(serializeResourceProperty(resourceProperty, blocksData));
                }
            }

            return binaryResourceBinding;
        }

        [[nodiscard]] BinaryPushConstantReferenceProperty serializePushConstantReferenceProperty(PushConstantReferenceProperty const& referenceProperty, LitlShaderBlocksData& blocksData) noexcept
        {
            BinaryPushConstantReferenceProperty binaryReferenceProperty{
                .name = BinaryBlockFile::serializeString(referenceProperty.name, blocksData.stringMap),
                .hashedName = referenceProperty.hashedName.value,
                .offset = referenceProperty.offset,
                .size = referenceProperty.sizeBytes,
                .stride = referenceProperty.stride
            };

            if (!referenceProperty.properties.empty())
            {
                binaryReferenceProperty.resourceProperties = nextBinaryRecordGrouping(sizeof(BinaryResourceProperty), referenceProperty.properties.size(), blocksData.runningResourcePropertyOffset);

                for (auto& resourceProperty : referenceProperty.properties)
                {
                    blocksData.resourceProperties.push_back(serializeResourceProperty(resourceProperty, blocksData));
                }
            }

            return binaryReferenceProperty;
        }

        [[nodiscard]] BinaryPushConstantRange serializePushConstant(PushConstantRange const& pushConstant, LitlShaderBlocksData& blocksData) noexcept
        {
            BinaryPushConstantRange binaryPushConstant{
                .offset = pushConstant.offset,
                .size = pushConstant.sizeBytes
            };

            if (!pushConstant.properties.empty())
            {
                binaryPushConstant.resourceProperties = nextBinaryRecordGrouping(sizeof(BinaryResourceProperty), pushConstant.properties.size(), blocksData.runningResourcePropertyOffset);

                for (auto& resourceProperty : pushConstant.properties)
                {
                    blocksData.resourceProperties.push_back(serializeResourceProperty(resourceProperty, blocksData));
                }
            }

            if (!pushConstant.referenceProperties.empty())
            {
                binaryPushConstant.referenceProperties = nextBinaryRecordGrouping(sizeof(BinaryPushConstantReferenceProperty), pushConstant.referenceProperties.size(), blocksData.runningPushConstantReferencePropertyOffset);

                for (auto& referenceProperty : pushConstant.referenceProperties)
                {
                    blocksData.pushConstantReferenceProperties.push_back(serializePushConstantReferenceProperty(referenceProperty, blocksData));
                }
            }

            return binaryPushConstant;
        }

        [[nodiscard]] BinaryShaderInputOutputVariable serializeShaderInputOutputVariable(ShaderInputOutputVariable const& inoutVariable, LitlShaderBlocksData& blocksData) noexcept
        {
            return BinaryShaderInputOutputVariable{
                .name = BinaryBlockFile::serializeString(inoutVariable.name, blocksData.stringMap),
                .variable = serializeShaderVariable(inoutVariable.variable),
                .location = inoutVariable.location,
                .componentCount = inoutVariable.componentCount
            };
        }

        void serializeBinaryEntryPoint(EntryPointReflection const& entryPoint, BinaryEntryPointReflection& binaryEntryPoint, LitlShaderBlocksData& blocksData) noexcept
        {
            binaryEntryPoint.entryPoint = BinaryBlockFile::serializeString(entryPoint.entryPoint, blocksData.stringMap);
            binaryEntryPoint.stage = static_cast<uint32_t>(entryPoint.stage);

            if (!entryPoint.resources.empty())
            {
                binaryEntryPoint.resources = nextBinaryRecordGrouping(sizeof(BinaryResourceBinding), entryPoint.resources.size(), blocksData.runningResourceBindingOffset);

                for (auto& resourceBinding : entryPoint.resources)
                {
                    blocksData.resourceBindings.push_back(serializeResourceBinding(resourceBinding, blocksData));
                }
            }

            if (!entryPoint.pushConstants.empty())
            {
                binaryEntryPoint.pushConstants = nextBinaryRecordGrouping(sizeof(BinaryPushConstantRange), entryPoint.pushConstants.size(), blocksData.runningPushConstantsOffset);

                for (auto& pushConstant : entryPoint.pushConstants)
                {
                    blocksData.pushConstants.push_back(serializePushConstant(pushConstant, blocksData));
                }
            }

            if (!entryPoint.vertexInputs.empty())
            {
                binaryEntryPoint.vertexInputs = nextBinaryRecordGrouping(sizeof(BinaryShaderInputOutputVariable), entryPoint.vertexInputs.size(), blocksData.runningVertexFragmentInputOutputOffset);

                for (auto& vertexInput : entryPoint.vertexInputs)
                {
                    blocksData.shaderInputOutput.push_back(serializeShaderInputOutputVariable(vertexInput, blocksData));
                }
            }

            if (!entryPoint.fragmentOutputs.empty())
            {
                binaryEntryPoint.fragmentOutputs = nextBinaryRecordGrouping(sizeof(BinaryShaderInputOutputVariable), entryPoint.fragmentOutputs.size(), blocksData.runningVertexFragmentInputOutputOffset);

                for (auto& fragmentOutput : entryPoint.fragmentOutputs)
                {
                    blocksData.shaderInputOutput.push_back(serializeShaderInputOutputVariable(fragmentOutput, blocksData));
                }
            }

            if (entryPoint.computeInfo.has_value())
            {
                binaryEntryPoint.computeInfo.localSizeX = entryPoint.computeInfo.value().localSizeX;
                binaryEntryPoint.computeInfo.localSizeY = entryPoint.computeInfo.value().localSizeY;
                binaryEntryPoint.computeInfo.localSizeZ = entryPoint.computeInfo.value().localSizeZ;
            }
        }
    }

    bool LitlShader::serialize(ShaderIntermediateData const& shader, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        LitlShader litlShader{};
        LitlShaderBlocksData blocksData{};
        auto& reflection = shader.getReflection();

        std::vector<BlockDataDescriptor> blockDataTable; blockDataTable.reserve(MaxBlocks);
        litlShader.addDefaultBlockDescriptors(blockDataTable);

        for (auto& entryPoint : reflection.entryPoints)
        {
            blocksData.entryPoints.emplace_back();
            serializeBinaryEntryPoint(entryPoint, blocksData.entryPoints.back(), blocksData);
        }

        if (!addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 0], BlockIds::EntryPoints, sizeof(BinaryEntryPointReflection), as_byte_span(blocksData.entryPoints), error) ||
            !addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 1], BlockIds::ResourceBindings, sizeof(BinaryResourceBinding), as_byte_span(blocksData.resourceBindings), error) ||
            !addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 2], BlockIds::PushConstants, sizeof(BinaryPushConstantRange), as_byte_span(blocksData.pushConstants), error) ||
            !addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 3], BlockIds::PushConstantReferenceProperties, sizeof(BinaryPushConstantReferenceProperty), as_byte_span(blocksData.pushConstantReferenceProperties), error) ||
            !addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 4], BlockIds::VertexFragmentInputOutput, sizeof(BinaryShaderInputOutputVariable), as_byte_span(blocksData.shaderInputOutput), error) ||
            !addDataBlockDescriptor(blockDataTable, &litlShader.descriptors[BinaryBlockFile::DefaultBlocks::DefaultBlocksCount + 5], BlockIds::ResourceProperties, sizeof(BinaryResourceProperty), as_byte_span(blocksData.resourceProperties), error))
        {
            // ... too many blocks set by addDataBlockDescriptor ...
            return false;
        }

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

        litlShader.header.magic = Identity.magic;
        litlShader.header.versionMajor = Identity.versionMajor;
        litlShader.header.versionMinor = Identity.versionMinor;
        litlShader.header.contentHash = 0ull;         // calculated further on
        litlShader.header.totalBytes = 0u;            // calculated further on
        litlShader.header.blockCount = static_cast<uint32_t>(blockDataTable.size());
        litlShader.header.descriptorsOffset = sizeof(Header);
        litlShader.header.blocksOffset = litlShader.header.descriptorsOffset + (sizeof(BlockDescriptor) * litlShader.header.blockCount);
        litlShader.header.flags = 0u;

        // ---------------------------------------------------------------------------------
        // Populate BlockDescriptors

        litlShader.serializeDefaultBlocks(blockDataTable, blocksData.stringMap);

        uint64_t runningOffset = litlShader.header.blocksOffset;

        for (uint32_t i = 0; i < litlShader.header.blockCount; ++i)
        {
            auto& blockData = blockDataTable[i];
            serializeBlock(blockData, runningOffset);
        }

        litlShader.header.totalBytes = runningOffset;

        // ---------------------------------------------------------------------------------
        // Copy content to the provided data buffer

        serializeDataBuffer(litlShader, blockDataTable, data);

        return true;
    }

    bool LitlShader::deserialize(ShaderIntermediateData& shader, ErrorCode& error) const noexcept
    {
        // ... todo ...

        return true;
    }
}