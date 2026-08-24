#include <array>

#include "litl-core/constants.hpp"
#include "litl-core/logging/logging.hpp"
#include "litl-engine/objects/material/materialProperties.hpp"

namespace litl
{
    void MaterialProperties::configure(uint32_t slotSize) noexcept
    {
        if (!m_propertyBlocks.empty())
        {
            return;
        }

        m_slotBytes = slotSize;
        allocateBlock();
    }

    void MaterialProperties::allocateBlock() noexcept
    {
        m_propertyBlocks.push_back(std::make_unique<MaterialPropertyBlock>());
        auto& newBlock = m_propertyBlocks.back();

        newBlock->data.resize(m_slotBytes * SlotsPerBlock, std::byte{ 0 });
        newBlock->slots.resize(SlotsPerBlock);
        newBlock->vacantSlotCount = SlotsPerBlock;
        newBlock->activeSlotCount = 0u;
        newBlock->isDirty = false;

        m_vacantSlotCount += SlotsPerBlock;
    }

    bool MaterialPropertyBlock::acquireSlot(uint32_t& localSlotIndex) noexcept
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
        vacantSlotCount = (vacantSlotCount == 0u ? 0u : (vacantSlotCount - 1u));

        return true;
    }

    uint32_t MaterialProperties::allocateSlot() noexcept
    {
        uint32_t localSlotIndex = 0u;

        for (uint32_t i = 0u; i < static_cast<uint32_t>(m_propertyBlocks.size()); ++i)
        {
            if (m_propertyBlocks[i]->acquireSlot(localSlotIndex))
            {
                m_vacantSlotCount = (m_vacantSlotCount == 0u ? 0u : m_vacantSlotCount - 1u);
                return ((SlotsPerBlock * i) + localSlotIndex);
            }
        }

        allocateBlock();

        if (m_propertyBlocks.back()->acquireSlot(localSlotIndex))
        {
            m_vacantSlotCount = (m_vacantSlotCount == 0u ? 0u : m_vacantSlotCount - 1u);
            return ((SlotsPerBlock * static_cast<uint32_t>(m_propertyBlocks.size() - 1)) + localSlotIndex);
        }
        else
        {
            // This shouldn't be possible, but log just in case ...
            logWarning("Failed to acquire new material slot.");
            return Constants::uint32_null_index;
        }
    }

    void MaterialProperties::markSlotActive(uint32_t slot, uint32_t frame) noexcept
    {
        uint32_t blockIndex, localSlotIndex;
        
        if (getBlockLocalSlot(slot, blockIndex, localSlotIndex))
        {
            m_propertyBlocks[blockIndex]->slots[localSlotIndex].lastActiveFrame = frame;
        }
    }

    void MaterialProperties::freeSlots(uint32_t frame) noexcept
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
                    if (slot.occupied && (slot.lastActiveFrame < frame))
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
        return (m_slotBytes * SlotsPerBlock * m_propertyBlocks.size());
    }

    bool MaterialProperties::getBlockLocalSlot(uint32_t slot, uint32_t& blockIndex, uint32_t& localSlot) const noexcept
    {
        if (static_cast<size_t>(slot) >= (m_propertyBlocks.size() * SlotsPerBlock))
        {
            return false;
        }

        blockIndex = slot / SlotsPerBlock;
        localSlot = slot % SlotsPerBlock;

        return true;
    }

    bool MaterialProperties::setData(StringId property, uint32_t propertyBytes, void* propertyData, uint32_t slot) noexcept
    {
        uint32_t blockIndex, localSlotIndex;

        if (!getBlockLocalSlot(slot, blockIndex, localSlotIndex))
        {
            return false;
        }

        auto findPropertyOffset = m_propertyOffsets.find(property);

        if (findPropertyOffset == m_propertyOffsets.end())
        {
            return false;
        }

        auto& block = m_propertyBlocks[blockIndex];
        auto* blockData = block->data.data();
        auto blockSlotPropertyOffset = (localSlotIndex * m_slotBytes) + findPropertyOffset->second;

        memcpy(blockData + blockSlotPropertyOffset, propertyData, static_cast<size_t>(propertyBytes));

        block->isDirty = true;

        return true;
    }

    bool MaterialProperties::setBool(StringId property, bool value, uint32_t slot) noexcept
    {
        // bool on the cpu is 1 byte, but on the gpu it is 4 bytes.
        std::array<uint8_t, 4> fullsize{ uint8_t{ 0 }, uint8_t{ 0 }, uint8_t{ 0 }, uint8_t{ 0 } };
        fullsize[0] = (value ? uint8_t{ 1 } : uint8_t{ 0 });
        return setData(property, 4u, fullsize.data(), slot);
    }

    bool MaterialProperties::setInt32(StringId property, int32_t value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(int32_t)), &value, slot);
    }

    bool MaterialProperties::setUint32(StringId property, uint32_t value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(uint32_t)), &value, slot);
    }

    bool MaterialProperties::setFloat(StringId property, float value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(float)), &value, slot);
    }

    bool MaterialProperties::setVec2(StringId property, vec2 value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(vec2)), &value, slot);
    }

    bool MaterialProperties::setVec3(StringId property, vec3 value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(vec3)), &value, slot);
    }

    bool MaterialProperties::setVec4(StringId property, vec4 const& value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(vec4)), static_cast<void*>(const_cast<vec4*>(&value)), slot);
    }

    bool MaterialProperties::setMat3(StringId property, mat3 const& value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(mat3)), static_cast<void*>(const_cast<mat3*>(&value)), slot);
    }

    bool MaterialProperties::setMat4(StringId property, mat4 const& value, uint32_t slot) noexcept
    {
        return setData(property, static_cast<uint32_t>(sizeof(mat4)), static_cast<void*>(const_cast<mat4*>(&value)), slot);
    }

}