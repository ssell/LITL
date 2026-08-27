#include <algorithm>
#include <cstring>

#include "litl-core/logging/logging.hpp"
#include "litl-core/math/common.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"

namespace litl
{
    namespace
    {
        [[nodiscard]] bool insertUniqueNonOverlappingProperties(std::vector<ResourceProperty> const& reflectedProperties, std::vector<ResourceProperty>& properties, StringIdMap<uint32_t>& propertyMap) noexcept
        {
            for (auto& reflectedProperty : reflectedProperties)
            {
                // Valid name
                if (reflectedProperty.hashedName == StringId{})
                {
                    continue;
                }

                // Dedupe
                if (propertyMap.find(reflectedProperty.hashedName) != propertyMap.end())
                {
                    continue;
                }

                // Range is open
                for (auto& preexistingProperty : properties)
                {
                    if (overlap(reflectedProperty.offset, (reflectedProperty.offset + reflectedProperty.sizePadded - 1u), preexistingProperty.offset, (preexistingProperty.offset + preexistingProperty.sizePadded - 1u)))
                    {
                        logWarning("MaterialProperties::configure encountered property block overlap between properties '", preexistingProperty.name, "' and '", reflectedProperty.name, "'");
                        return false;
                    }
                }

                propertyMap[reflectedProperty.hashedName] = static_cast<uint32_t>(properties.size());
                properties.push_back(reflectedProperty);
            }

            return true;
        }
    }

    bool MaterialProperties::configure(MaterialPropertyReflection const& reflectedProperties, uint32_t framesInFlight) noexcept
    {
        if (!m_propertyBlocks.empty())
        {
            // TODO update for shader hot reload
            return true;    // no action for now
        }

        if (reflectedProperties.sizeBytes == 0u)
        {
            logWarning("MaterialProperties::configure provided with reflected properties with element size of 0. Rejecting.");
            return false;
        }

        if (!insertUniqueNonOverlappingProperties(reflectedProperties.properties, m_properties, m_propertyMap))
        {
            m_properties.clear();
            m_propertyMap.clear();
            return false;
        }

        m_slotSizeBytes = reflectedProperties.sizeBytes;
        m_framesInFlight = framesInFlight;
        m_defaultPropertyBlob.resize(m_slotSizeBytes, std::byte{ 0 });
        m_frequentUpdateBlock.residents.reserve(SlotsPerBlock);

        allocateBlock();

        return true;
    }

    bool MaterialProperties::setDefaultPropertyBlob(std::span<std::byte const> defaultBlob) noexcept
    {
        if (defaultBlob.size() != m_defaultPropertyBlob.size())
        {
            logWarning("Attempting to set new default property blob for Material that does not match existing blob size.");
            return false;
        }

        m_defaultPropertyBlob.clear();
        m_defaultPropertyBlob.insert(m_defaultPropertyBlob.begin(), defaultBlob.begin(), defaultBlob.end());

        return true;
    }

    void MaterialProperties::setCurrentFrame(uint32_t currFrame) noexcept
    {
        m_currFrame = currFrame;
    }

    void MaterialProperties::allocateBlock() noexcept
    {
        m_propertyBlocks.push_back(std::make_unique<MaterialPropertyBlock>());
        auto& newBlock = m_propertyBlocks.back();

        newBlock->data.resize(m_slotSizeBytes * SlotsPerBlock, std::byte{ 0 });
        newBlock->slots.resize(SlotsPerBlock);
        newBlock->vacantSlotCount = SlotsPerBlock;
        newBlock->dirtyFrameCount = m_framesInFlight;
    }

    bool MaterialPropertyBlock::acquireSlot(uint32_t slotSize, uint32_t frame, uint32_t framesInFlight, uint32_t& localSlotIndex, uint32_t& localSlotVersion, std::span<std::byte const> defaultBlob) noexcept
    {
        if (vacantSlotCount == 0u)
        {
            return false;
        }

        for (localSlotIndex = 0u; localSlotIndex < MaterialProperties::SlotsPerBlock; ++localSlotIndex)
        {
            if (!slots[localSlotIndex].occupied)
            {
                break;
            }
        }

        if (localSlotIndex == MaterialProperties::SlotsPerBlock)
        {
            // Shouldn't get here ...
            return false;
        }

        slots[localSlotIndex].occupied = true;
        slots[localSlotIndex].lastActiveFrame = frame;

        localSlotVersion = ++slots[localSlotIndex].version;
        vacantSlotCount = (vacantSlotCount == 0u ? 0u : (vacantSlotCount - 1u));

        std::memcpy(data.data() + (slotSize * localSlotIndex), defaultBlob.data(), defaultBlob.size_bytes());
        dirtyFrameCount = framesInFlight;

        return true;
    }

    MaterialPropertySlotId MaterialProperties::allocateSlot() noexcept
    {
        if (m_slotSizeBytes == 0u)
        {
            // A material with no property block is legitimate but we can't allocate a slot for it.
            return {};
        }

        uint32_t localSlotIndex = 0u;
        uint32_t localSlotVersion = 0u;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            if (m_propertyBlocks[i]->acquireSlot(m_slotSizeBytes, m_currFrame, m_framesInFlight, localSlotIndex, localSlotVersion, m_defaultPropertyBlob))
            {
                return MaterialPropertySlotId{
                    .index = ((SlotsPerBlock * i) + localSlotIndex),
                    .version = localSlotVersion
                };
            }
        }

        allocateBlock();

        if (m_propertyBlocks.back()->acquireSlot(m_slotSizeBytes, m_currFrame, m_framesInFlight, localSlotIndex, localSlotVersion, m_defaultPropertyBlob))
        {
            return MaterialPropertySlotId{
                .index = ((SlotsPerBlock * static_cast<uint32_t>(m_propertyBlocks.size() - 1)) + localSlotIndex),
                .version = localSlotVersion
            };
        }
        else
        {
            // This shouldn't be possible, but log just in case ...
            logWarning("Failed to acquire new material slot.");
            return {};
        }
    }

    void MaterialProperties::markSlotActive(MaterialPropertySlotId slot) noexcept
    {
        uint32_t blockIndex, localSlotIndex, localSlotVersion;

        if (slot.isValid() && getBlockLocalSlot(slot, blockIndex, localSlotIndex, localSlotVersion, true))
        {
            m_propertyBlocks[blockIndex]->slots[localSlotIndex].lastActiveFrame = m_currFrame;
        }
    }

    void MaterialProperties::markAllBlocksDirty() noexcept
    {
        for (auto& propertyBlock : m_propertyBlocks)
        {
            propertyBlock->dirtyFrameCount = m_framesInFlight;
        }
    }

    void MaterialProperties::markSlotAsFrequentUpdate(MaterialPropertySlotId slot, bool isFrequent) noexcept
    {
        if (!slot.isValid())
        {
            return;
        }

        uint32_t blockIndex, localSlot, localSlotVersion;

        if (getBlockLocalSlot(slot, blockIndex, localSlot, localSlotVersion, true))
        {
            if (localSlotVersion != slot.version)
            {
                return;
            }

            auto& slotRef = m_propertyBlocks[blockIndex]->slots[localSlot];

            if (slotRef.isFrequent == isFrequent)
            {
                return;
            }

            if (isFrequent)
            {
                slotRef.isFrequent = true;
                slotRef.frequentGlobalSlot = static_cast<uint32_t>((m_propertyBlocks.size() * SlotsPerBlock) + m_frequentUpdateBlock.residents.size());
                m_frequentUpdateBlock.residents.push_back(slot.index);
            }
            else
            {
                slotRef.isFrequent = false;
                slotRef.frequentGlobalSlot = Constants::uint32_null_index;
                m_frequentUpdateBlock.removeResident(slot.index);
                m_propertyBlocks[blockIndex]->dirtyFrameCount = m_framesInFlight;       // The main block data on the GPU may be stale now. Mark dirty so it can be refreshed.
            }
        }
    }

    uint32_t MaterialProperties::getFrequentUpdateSlot(MaterialPropertySlotId slotId) noexcept
    {
        uint32_t blockIndex, localSlot, slotVersion;

        if (getBlockLocalSlot(slotId, blockIndex, localSlot, slotVersion, true))
        {
            auto& slot = m_propertyBlocks[blockIndex]->slots[localSlot];

            if (slot.isFrequent && slot.frequentGlobalSlot != Constants::uint32_null_index)
            {
                return slot.frequentGlobalSlot;
            }
        }

        return Constants::uint32_null_index;
    }

    void MaterialProperties::rebuildFrequentUpdateBlock() noexcept
    {
        if (m_frequentUpdateBlock.residents.empty())
        {
            return;
        }

        const uint32_t frequentUpdateBlockFirstIndex = static_cast<uint32_t>(m_propertyBlocks.size()) * SlotsPerBlock;
        uint32_t blockIndex, localSlot;

        m_frequentUpdateBlock.data.resize(m_frequentUpdateBlock.residents.size()* m_slotSizeBytes, std::byte{ 0 });

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_frequentUpdateBlock.residents.size()); ++i)
        {
            const auto resident = m_frequentUpdateBlock.residents[i];
            std::byte* residentDataPtr = getSlotDataPtr({ resident }, blockIndex, localSlot, false);

            if (residentDataPtr != nullptr)
            {
                m_propertyBlocks[blockIndex]->slots[localSlot].frequentGlobalSlot = frequentUpdateBlockFirstIndex + i;
                memcpy(m_frequentUpdateBlock.data.data() + (i * m_slotSizeBytes), residentDataPtr, m_slotSizeBytes);
            }
        }
    }

    bool MaterialProperties::gatherFrequentUpdateBlockPointer(MaterialPropertyBlockPointer& blockPointer) noexcept
    {
        if (m_frequentUpdateBlock.residents.empty())
        {
            return false;
        }

        blockPointer.sourcePtr = m_frequentUpdateBlock.data;
        blockPointer.blockOffset = static_cast<uint32_t>(m_propertyBlocks.size()) * SlotsPerBlock * m_slotSizeBytes;

        return true;
    }

    void MaterialProperties::freeSlots() noexcept
    {
        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            auto& block = m_propertyBlocks[i];

            for (uint32_t j = (m_currFrame % SlotExpirationFrames); j < SlotsPerBlock; j += SlotExpirationFrames)       // Stagger only check 1/8 slots per frame
            {
                auto& slot = block->slots[j];

                // If the slot is labelled active but hasn't been used, then mark it as vacant so it can be reused.
                if (slot.occupied && ((slot.lastActiveFrame + SlotExpirationFrames) < m_currFrame))
                {
                    // Reset the slot tracking
                    slot.occupied = false;
                    slot.version++;
                    block->vacantSlotCount++;

                    if (slot.isFrequent)
                    {
                        m_frequentUpdateBlock.removeResident((SlotsPerBlock * i) + j);
                        slot.isFrequent = false;
                    }
                }
            }
        }
    }

    uint32_t MaterialProperties::individualSlotMemoryRequirements() const noexcept
    {
        return m_slotSizeBytes;
    }

    size_t MaterialProperties::totalMemoryRequirements() const noexcept
    {
        return (m_slotSizeBytes * SlotsPerBlock * m_propertyBlocks.size()) + (m_slotSizeBytes * m_frequentUpdateBlock.residents.size());
    }

    uint32_t MaterialProperties::propertyCount() const noexcept
    {
        return static_cast<uint32_t>(m_properties.size());
    }

    void MaterialProperties::gatherDirtyBlocks(std::vector<MaterialPropertyBlockPointer>& dirtyBlocks) noexcept
    {
        dirtyBlocks.clear();
        dirtyBlocks.reserve(m_propertyBlocks.size());

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            if (m_propertyBlocks[i]->dirtyFrameCount > 0u)
            {
                dirtyBlocks.push_back(MaterialPropertyBlockPointer{
                    .sourcePtr = m_propertyBlocks[i]->data,
                    .blockOffset = i * SlotsPerBlock * m_slotSizeBytes
                });
            }
        }
    }

    void MaterialProperties::clearDirtyBlocks() noexcept
    {
        for (auto& propertyBlock : m_propertyBlocks)
        {
            propertyBlock->dirtyFrameCount = (propertyBlock->dirtyFrameCount == 0u ? 0u : propertyBlock->dirtyFrameCount - 1u);
        }
    }

    bool MaterialProperties::getBlockLocalSlot(MaterialPropertySlotId slotId, uint32_t& blockIndex, uint32_t& localSlot, uint32_t& localSlotVersion, bool validateVersion) const noexcept
    {
        if (static_cast<size_t>(slotId.index) >= (m_propertyBlocks.size() * SlotsPerBlock))
        {
            return false;
        }

        blockIndex = slotId.index / SlotsPerBlock;
        localSlot = slotId.index % SlotsPerBlock;
        localSlotVersion = m_propertyBlocks[blockIndex]->slots[localSlot].version;

        return !validateVersion || (slotId.version == localSlotVersion);
    }

    std::byte* MaterialProperties::getSlotDataPtr(MaterialPropertySlotId slotId, uint32_t& blockIndex, uint32_t& localSlot, bool validateVersion) const noexcept
    {
        uint32_t localSlotVersion;

        if (getBlockLocalSlot(slotId, blockIndex, localSlot, localSlotVersion, validateVersion))
        {
            return (m_propertyBlocks[blockIndex]->data.data() + (localSlot * m_slotSizeBytes));
        }

        return nullptr;
    }

    ResourceProperty const* MaterialProperties::getReflectedProperty(StringId property) const noexcept
    {
        auto find = m_propertyMap.find(property);

        if (find == m_propertyMap.end())
        {
            return nullptr;
        }

        return &m_properties[find->second];
    }

    bool MaterialProperties::setData(uint32_t propertyOffset, uint32_t propertySize, void const* propertyData, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        if (defaultValue)
        {
            if ((propertyOffset + propertySize) > static_cast<uint32_t>(m_defaultPropertyBlob.size()))
            {
                return false;       // out-of-bounds
            }

            std::memcpy(m_defaultPropertyBlob.data() + propertyOffset, propertyData, propertySize);

            return true;
        }
        else
        {
            if (!slot.isValid())
            {
                return false;       // invalid slot
            }

            uint32_t blockIndex, localSlot;
            std::byte* slotDataPtr = getSlotDataPtr(slot, blockIndex, localSlot, true);

            if (slotDataPtr == nullptr)
            {
                return false;
            }

            std::memcpy(slotDataPtr + propertyOffset, propertyData, static_cast<size_t>(propertySize));
            auto& slotRef = m_propertyBlocks[blockIndex]->slots[localSlot];

            if (!slotRef.isFrequent)
            {
                // Mark the whole block dirty if this is not a slot that is getting copied in the FrequentUpdateBlock.
                m_propertyBlocks[blockIndex]->dirtyFrameCount = m_framesInFlight;
            }

            return true;
        }
    }

    bool MaterialProperties::setBool(StringId property, bool value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Bool) ||
            (reflectedProperty->variable.scalarSize != sizeof(uint32_t)) ||
            (reflectedProperty->variable.componentCount != 1u))
        {
            return false;
        }

        // bool on the cpu is 1 byte, but on the gpu it is 4 bytes.
        const uint32_t value32 = (value ? 1u : 0u);
        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value32, slot, defaultValue);
    }

    bool MaterialProperties::setInt32(StringId property, int32_t value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Integer) ||
            (reflectedProperty->variable.scalarSize != sizeof(int32_t)) ||
            (reflectedProperty->variable.componentCount != 1u))
        {
            return false;
        }

        if (has_any(reflectedProperty->variable.flag, ShaderVariableFlagBits::Unsigned))
        {
            // It is actually an unsigned integer.
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Integer) || 
            (reflectedProperty->variable.scalarSize != sizeof(uint32_t)) ||
            (reflectedProperty->variable.componentCount != 1u))
        {
            return false;
        }

        if (!has_any(reflectedProperty->variable.flag, ShaderVariableFlagBits::Unsigned))
        {
            // It is actually a signed integer.
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setFloat(StringId property, float value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 1u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setDouble(StringId property, double value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(double)) ||
            (reflectedProperty->variable.componentCount != 1u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setVec2(StringId property, vec2 value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(vec2) == (sizeof(float) * 2));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 2u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setVec3(StringId property, vec3 value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(vec3) == (sizeof(float) * 3));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 3u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(vec4) == (sizeof(float) * 4));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 4u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setColor(StringId property, color const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(color) == (sizeof(float) * 4));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            ((reflectedProperty->variable.componentCount != 3u) && (reflectedProperty->variable.componentCount != 4u)))      // accept both RGB and RBA (float3 and float4)
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    bool MaterialProperties::setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(mat3) == (sizeof(float) * 9));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 9u))
        {
            return false;
        }

        const uint32_t matStride = reflectedProperty->variable.matrixStride;

        if (matStride == 12u)
        {
            return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
        }
        else if (matStride == 16u)      // 4 bytes padding at the end of each column
        {
            const uint32_t colSize = static_cast<uint32_t>(sizeof(float) * 3);

            return
                setData(reflectedProperty->offset + (matStride * 0u), colSize, value[0], slot, defaultValue) &&
                setData(reflectedProperty->offset + (matStride * 1u), colSize, value[1], slot, defaultValue) &&
                setData(reflectedProperty->offset + (matStride * 2u), colSize, value[2], slot, defaultValue);
        }
        else
        {
            return false;
        }
    }

    bool MaterialProperties::setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        static_assert(sizeof(mat4) == (sizeof(float) * 16));

        if ((reflectedProperty->variable.scalarType != ShaderScalarType::Float) ||
            (reflectedProperty->variable.scalarSize != sizeof(float)) ||
            (reflectedProperty->variable.componentCount != 16u) ||
            (reflectedProperty->variable.matrixStride != 16u))
        {
            return false;
        }

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot, defaultValue);
    }

    void FrequentUpdateBlock::removeResident(uint32_t slot) noexcept
    {
        std::erase(residents, slot);
    }
}