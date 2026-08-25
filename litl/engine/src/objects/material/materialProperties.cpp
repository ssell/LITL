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
                    if (overlap(reflectedProperty.offset, (reflectedProperty.offset + reflectedProperty.sizePadded), preexistingProperty.offset, (preexistingProperty.offset + preexistingProperty.sizePadded)))
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
    bool MaterialProperties::configure(MaterialPropertyReflection const& reflectedProperties) noexcept
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

        m_elementSizeBytes = reflectedProperties.sizeBytes;

        allocateBlock();

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

        newBlock->data.resize(m_elementSizeBytes * SlotsPerBlock, std::byte{ 0 });
        newBlock->slots.resize(SlotsPerBlock);
        newBlock->vacantSlotCount = SlotsPerBlock;
        newBlock->activeSlotCount = 0u;
        newBlock->isDirty = false;
    }

    bool MaterialPropertyBlock::acquireSlot(uint32_t slotSize, uint32_t frame, uint32_t& localSlotIndex, uint32_t& localSlotVersion) noexcept
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

        // Make sure the memory is cleared. TODO in the future use the material default values ...
        std::memset(data.data() + (slotSize * localSlotIndex), 0, slotSize);
        isDirty = true;

        return true;
    }

    MaterialPropertySlotId MaterialProperties::allocateSlot() noexcept
    {
        if (m_elementSizeBytes == 0u)
        {
            // A material with no property block is legitimate but we can't allocate a slot for it.
            return {};
        }

        uint32_t localSlotIndex = 0u;
        uint32_t localSlotVersion = 0u;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            if (m_propertyBlocks[i]->acquireSlot(m_elementSizeBytes, m_currFrame, localSlotIndex, localSlotVersion))
            {
                return MaterialPropertySlotId{
                    .slot = ((SlotsPerBlock * i) + localSlotIndex),
                    .version = localSlotVersion
                };
            }
        }

        allocateBlock();

        if (m_propertyBlocks.back()->acquireSlot(m_elementSizeBytes, m_currFrame, localSlotIndex, localSlotVersion))
        {
            return MaterialPropertySlotId{
                .slot = ((SlotsPerBlock * static_cast<uint32_t>(m_propertyBlocks.size() - 1)) + localSlotIndex),
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
        
        if (slot.isValid() &&            
            getBlockLocalSlot(slot.slot, blockIndex, localSlotIndex, localSlotVersion) && 
            (slot.version == localSlotVersion))
        {
            m_propertyBlocks[blockIndex]->slots[localSlotIndex].lastActiveFrame = m_currFrame;
        }
    }

    void MaterialProperties::calculateActiveSlotCounts() noexcept
    {
        for (auto& block : m_propertyBlocks)
        {
            block->activeSlotCount = 0u;

            for (auto& slot : block->slots)
            {
                block->activeSlotCount += (slot.occupied && (slot.lastActiveFrame == m_currFrame)) ? 1u : 0u;
            }
        }
    }

    void MaterialProperties::freeSlots() noexcept
    {
        for (auto& block : m_propertyBlocks)
        {
            auto occupiedSlots = SlotsPerBlock - block->vacantSlotCount;

            // Are there inactive slots?
            if (block->activeSlotCount < occupiedSlots)
            {
                for (uint32_t i = 0u; i < SlotsPerBlock; ++i)
                {
                    auto& slot = block->slots[i];

                    // If the slot is labelled active but hasn't been used, then mark it as vacant so it can be reused.
                    if (slot.occupied && ((slot.lastActiveFrame + SlotExpirationFrames) < m_currFrame))
                    {
                        // Reset the slot tracking
                        slot.occupied = false;
                        slot.version++;
                        block->vacantSlotCount++;
                    }
                }
            }
        }
    }

    uint32_t MaterialProperties::individualSlotMemoryRequirements() const noexcept
    {
        return m_elementSizeBytes;
    }

    size_t MaterialProperties::totalMemoryRequirements() const noexcept
    {
        return (m_elementSizeBytes * SlotsPerBlock * m_propertyBlocks.size());
    }

    bool MaterialProperties::getBlockLocalSlot(uint32_t slot, uint32_t& blockIndex, uint32_t& localSlot, uint32_t& localSlotVersion) const noexcept
    {
        if (static_cast<size_t>(slot) >= (m_propertyBlocks.size() * SlotsPerBlock))
        {
            return false;
        }

        blockIndex = slot / SlotsPerBlock;
        localSlot = slot % SlotsPerBlock;
        localSlotVersion = m_propertyBlocks[blockIndex]->slots[localSlot].version;

        return true;
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

    bool MaterialProperties::setData(uint32_t propertyOffset, uint32_t propertySize, void const* propertyData, MaterialPropertySlotId slot) noexcept
    {
        if (!slot.isValid())
        {
            return false;
        }

        uint32_t blockIndex, localSlotIndex, localSlotVersion;

        if (!getBlockLocalSlot(slot.slot, blockIndex, localSlotIndex, localSlotVersion))
        {
            return false;
        }

        if ((slot.version != localSlotVersion))
        {
            return false;
        }

        auto& block = m_propertyBlocks[blockIndex];
        auto* blockData = block->data.data();
        const auto blockSlotPropertyOffset = (localSlotIndex * m_elementSizeBytes) + propertyOffset;

        LITL_ASSERT_MSG(((propertyOffset + propertySize) <= m_elementSizeBytes), "Material setData out-of-bounds of property", false);
        LITL_ASSERT_MSG((static_cast<size_t>(blockSlotPropertyOffset + propertySize) <= block->data.size()), "Material setData out-of-bounds of block", false);

        memcpy(blockData + blockSlotPropertyOffset, propertyData, static_cast<size_t>(propertySize));

        block->isDirty = true;

        return true;
    }

    bool MaterialProperties::setBool(StringId property, bool value, MaterialPropertySlotId slot) noexcept
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
        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value32, slot);
    }

    bool MaterialProperties::setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

    bool MaterialProperties::setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept
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
            return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
        }
        else if (matStride == 16u)      // 4 bytes padding at the end of each column
        {
            const uint32_t colSize = static_cast<uint32_t>(sizeof(float) * 3);

            return
                setData(reflectedProperty->offset + (matStride * 0u), colSize, value[0], slot) &&
                setData(reflectedProperty->offset + (matStride * 1u), colSize, value[1], slot) &&
                setData(reflectedProperty->offset + (matStride * 2u), colSize, value[2], slot);
        }
        else
        {
            return false;
        }
    }

    bool MaterialProperties::setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot) noexcept
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

        return setData(reflectedProperty->offset, reflectedProperty->variable.scalarSize * reflectedProperty->variable.componentCount, &value, slot);
    }

}