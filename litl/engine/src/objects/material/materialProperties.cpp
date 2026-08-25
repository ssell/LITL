#include <array>

#include "litl-core/logging/logging.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"

namespace litl
{
    void MaterialProperties::configure(MaterialPropertyReflection const& reflectedProperties) noexcept
    {
        if (!m_propertyBlocks.empty())
        {
            return;
        }

        m_reflectedProperties = reflectedProperties;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_reflectedProperties.properties.size()); ++i)
        {
            m_propertyMap[m_reflectedProperties.properties[i].hashedName] = i;
        }

        allocateBlock();
    }

    void MaterialProperties::setCurrentFrame(uint32_t currFrame) noexcept
    {
        m_currFrame = currFrame;
    }

    void MaterialProperties::allocateBlock() noexcept
    {
        m_propertyBlocks.push_back(std::make_unique<MaterialPropertyBlock>());
        auto& newBlock = m_propertyBlocks.back();

        newBlock->data.resize(m_reflectedProperties.sizeBytes * SlotsPerBlock, std::byte{ 0 });
        newBlock->slots.resize(SlotsPerBlock);
        newBlock->vacantSlotCount = SlotsPerBlock;
        newBlock->activeSlotCount = 0u;
        newBlock->isDirty = false;

        m_vacantSlotCount += SlotsPerBlock;
    }

    bool MaterialPropertyBlock::acquireSlot(uint32_t& localSlotIndex, uint32_t& localSlotVersion) noexcept
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
        localSlotVersion = ++slots[localSlotIndex].version;
        vacantSlotCount = (vacantSlotCount == 0u ? 0u : (vacantSlotCount - 1u));

        return true;
    }

    MaterialPropertySlotId MaterialProperties::allocateSlot() noexcept
    {
        uint32_t localSlotIndex = 0u;
        uint32_t localSlotVersion = 0u;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            if (m_propertyBlocks[i]->acquireSlot(localSlotIndex, localSlotVersion))
            {
                m_vacantSlotCount = (m_vacantSlotCount == 0u ? 0u : m_vacantSlotCount - 1u);

                return MaterialPropertySlotId{
                    .slot = ((SlotsPerBlock * i) + localSlotIndex),
                    .version = localSlotVersion
                };
            }
        }

        allocateBlock();

        if (m_propertyBlocks.back()->acquireSlot(localSlotIndex, localSlotVersion))
        {
            m_vacantSlotCount = (m_vacantSlotCount == 0u ? 0u : m_vacantSlotCount - 1u);

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

    void MaterialProperties::freeSlots() noexcept
    {
        for (auto& block : m_propertyBlocks)
        {
            auto occupiedSlots = SlotsPerBlock - block->vacantSlotCount;

            // Are there inactive slots?
            if (block->activeSlotCount < occupiedSlots)
            {
                for (auto& slot : block->slots)
                {
                    // If the slot is labelled active but hasn't been used, then mark it as vacant so it can be reused.
                    if (slot.occupied && (slot.lastActiveFrame < m_currFrame))
                    {
                        slot.occupied = false;
                        block->vacantSlotCount++;
                        m_vacantSlotCount++;
                    }
                }
            }
        }
    }

    size_t MaterialProperties::totalMemoryRequirements() const noexcept
    {
        return (m_reflectedProperties.sizeBytes * SlotsPerBlock * m_propertyBlocks.size());
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

        return &m_reflectedProperties.properties[find->second];
    }

    bool MaterialProperties::setData(StringId property, uint32_t propertyOffset, uint32_t propertySize, void const* propertyData, MaterialPropertySlotId slot) noexcept
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
        const auto blockSlotPropertyOffset = (localSlotIndex * m_reflectedProperties.sizeBytes) + propertyOffset;

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

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Bool)
        {
            return false;
        }

        if (reflectedProperty->variable.size != sizeof(uint32_t))
        {
            return false;
        }

        // bool on the cpu is 1 byte, but on the gpu it is 4 bytes.
        const uint32_t value32 = (value ? 1u : 0u);
        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value32, slot);
    }

    bool MaterialProperties::setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Integer)
        {
            return false;
        }

        if (has_any(reflectedProperty->variable.flag, ShaderVariableFlagBits::Unsigned))
        {
            // It is actually an unsigned integer.
            return false;
        }

        if (reflectedProperty->size != sizeof(int32_t))
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Integer)
        {
            return false;
        }

        if (!has_any(reflectedProperty->variable.flag, ShaderVariableFlagBits::Unsigned))
        {
            // It is actually a signed integer.
            return false;
        }

        if (reflectedProperty->size != sizeof(uint32_t))
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(double))
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        if (reflectedProperty->variable.componentCount != 2u)
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        if (reflectedProperty->variable.componentCount != 3u)
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        if (reflectedProperty->variable.componentCount != 4u)
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

    bool MaterialProperties::setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept
    {
        auto* reflectedProperty = getReflectedProperty(property);

        if (reflectedProperty == nullptr)
        {
            return false;
        }

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        if (reflectedProperty->variable.componentCount != 9u)
        {
            return false;
        }

        const uint32_t matStride = reflectedProperty->variable.matrixStride;

        if (matStride == 12u)
        {
            return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
        }
        else if (matStride == 16u)      // 4 bytes padding at the end of each column
        {
            const uint32_t colSize = static_cast<uint32_t>(sizeof(float) * 3);

            return
                setData(property, reflectedProperty->offset + (matStride * 0u), colSize, value[0], slot) &&
                setData(property, reflectedProperty->offset + (matStride * 1u), colSize, value[1], slot) &&
                setData(property, reflectedProperty->offset + (matStride * 2u), colSize, value[2], slot);
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

        if (reflectedProperty->variable.scalarType != ShaderScalarType::Float)
        {
            return false;
        }

        if (reflectedProperty->size != sizeof(float))
        {
            return false;
        }

        if (reflectedProperty->variable.componentCount != 16u)
        {
            return false;
        }

        return setData(property, reflectedProperty->offset, reflectedProperty->size, &value, slot);
    }

}