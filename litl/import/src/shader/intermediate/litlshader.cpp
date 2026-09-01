#include <array>
#include <type_traits>
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

        static_assert(sizeof(RecordGBinaryRecordGroupingrouping) == 16);
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
            BinaryRecordGrouping resourceProperties{};
            uint32_t padding{ 0u };
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
            uint32_t variable{ 0u };
            uint32_t location{ 0u };
            uint32_t componentCount{ 0u };
        };

        static_assert(sizeof(BinaryShaderInputOutputVariable) == 32);
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

        [[nodiscard]] BinaryResourceProperty serializeResourceProperty(ResourceProperty const& resourceProperty, BinaryBlockFile::StringMap& stringMap) noexcept
        {
            return BinaryResourceProperty {
                .name = BinaryBlockFile::serializeString(resourceProperty.name, stringMap),
                .hashedName = resourceProperty.hashedName.value,
                .variable = serializeShaderVariable(resourceProperty.variable),
                .offset = resourceProperty.offset,
                .size = resourceProperty.size,
                .sizePadded = resourceProperty.sizePadded
            };
        }

        [[nodicard]] BinaryResourceBinding serializeResourceBinding(ResourceBinding const& resourceBinding, std::vector<BinaryResourceProperty>& binaryResourceProperties, uint64_t& runningResourcePropertyOffset, BinaryBlockFile::StringMap& stringMap) noexcept
        {
            BinaryResourceBinding binaryResourceBinding{
                        .name = BinaryBlockFile::serializeString(resourceBinding.name, stringMap),
                        .hashedName = resourceBinding.hashedName.value,
                        .type = static_cast<uint32_t>(resourceBinding.type),
                        .set = resourceBinding.set,
                        .binding = resourceBinding.binding,
                        .arraySize = resourceBinding.arraySize
            };

            if (!resourceBinding.properties.empty())
            {
                binaryResourceBinding.resourceProperties.count = resourceBinding.properties.size();
                binaryResourceBinding.resourceProperties.firstOffset = runningResourcePropertyOffset;
                runningResourcePropertyOffset += (sizeof(BinaryResourceProperty) * resourceBinding.properties.size());

                for (auto& resourceProperty : resourceBinding.properties)
                {
                    binaryResourceProperties.push_back(serializeResourceProperty(resourceProperty, stringMap));
                }
            }
        }
    }

    bool LitlShader::serialize(ShaderIntermediateData const& shader, std::vector<std::byte>& data, ErrorCode& error) noexcept
    {
        LitlShader litlShader{};
        StringMap stringMap{};

        std::vector<BlockDataDescriptor> blockDataTable; blockDataTable.reserve(MaxBlocks);
        litlShader.addDefaultBlockDescriptors(blockDataTable);

        auto& reflection = shader.getReflection();

        std::vector<BinaryResourceProperty> resourceProperties;                             // 'RESP' (RESource Properties)
        std::vector<BinaryShaderInputOutputVariable> shaderInputOutput;                     // 'VFIO' (Vertex Fragment Input/Output)
        std::vector<BinaryPushConstantReferenceProperty> pushConstantReferenceProperties;   // 'PURP' (PUsh constant Reference Properties)
        std::vector<BinaryPushConstantRange> pushConstants;                                 // 'PUSH' (PUSH constants)
        std::vector<BinaryResourceBinding> resourceBindings;                                // 'RESB' (RESource Bindings)
        std::vector<BinaryEntryPointReflection> entryPoints;                                // 'ENTR' (ENTRy points)

        uint64_t runningResourcePropertyOffset = 0ull;
        uint64_t runningResourceBindingOffset = 0ull;
        uint64_t runningPushConstantsOffset = 0ull;
        uint64_t runningVertexFragmentInputOutputOffset = 0ull;

        for (auto& entryPoint : reflection.entryPoints)
        {
            BinaryEntryPointReflection binaryEntryPoint{
                .entryPoint = BinaryBlockFile::serializeString(entryPoint.entryPoint, stringMap),
                .stage = static_cast<uint32_t>(entryPoint.stage)
            };

            if (!entryPoint.resources.empty())
            {
                binaryEntryPoint.resources = nextBinaryRecordGrouping(sizeof(BinaryResourceBinding), entryPoint.resources.size(), runningResourceBindingOffset);

                for (auto& resourceBinding : entryPoint.resources)
                {
                    resourceBindings.push_back(serializeResourceBinding(resourceBinding, resourceProperties, runningResourcePropertyOffset, stringMap));
                }
            }

            if (!entryPoint.pushConstants.empty())
            {
                binaryEntryPoint.pushConstants = nextBinaryRecordGrouping(sizeof(BinaryPushConstantRange), entryPoint.pushConstants.size(), runningPushConstantsOffset);

                for (auto& pushConstant : entryPoint.pushConstants)
                {
                    pushConstants.push_back(serializePushConstants());
                }
            }

            if (!entryPoint.vertexInputs.empty())
            {
                binaryEntryPoint.vertexInputs = nextBinaryRecordGrouping(sizeof(BinaryShaderInputOutputVariable), entryPoint.vertexInputs.size(), runningVertexFragmentInputOutputOffset);

                for (auto& vertexInput : entryPoint.vertexInputs)
                {
                    shaderInputOutput.push_back(serializeShaderInputOutputVariable());
                }
            }

            if (!entryPoint.fragmentOutputs.empty())
            {
                binaryEntryPoint.fragmentOutputs = nextBinaryRecordGrouping(sizeof(BinaryShaderInputOutputVariable), entryPoint.fragmentOutputs.size(), runningVertexFragmentInputOutputOffset);

                for (auto& fragmentOutput : entryPoint.fragmentOutputs)
                {
                    shaderInputOutput.push_back(serializeShaderInputOutputVariable());
                }
            }

            if (entryPoint.computeInfo.has_value())
            {
                binaryEntryPoint.computeInfo.localSizeX = entryPoint.computeInfo.value().localSizeX;
                binaryEntryPoint.computeInfo.localSizeY = entryPoint.computeInfo.value().localSizeY;
                binaryEntryPoint.computeInfo.localSizeZ = entryPoint.computeInfo.value().localSizeZ;
            }

            entryPoints.push_back(binaryEntryPoint);
        }

        return true;
    }

    bool LitlShader::deserialize(ShaderIntermediateData& shader, ErrorCode& error) const noexcept
    {
        // ... todo ...

        return true;
    }
}