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
        m_propertyBlocks.emplace_back();
        auto& newBlock = m_propertyBlocks.back();

        newBlock->data.resize(m_slotBytes * SlotsPerBlock, std::byte{ 0 });
        newBlock->slotLastActive.resize(SlotsPerBlock);
        newBlock->vacantSlotCount = SlotsPerBlock;
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
            if (!slotLastActive[localSlotIndex].active)
            {
                break;
            }
        }

        if (localSlotIndex == MaterialProperties::SlotsPerBlock)
        {
            // Shouldn't get here ...
            return false;
        }

        slotLastActive[localSlotIndex].active = true;
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
                return ((SlotsPerBlock * i) + localSlotIndex);
            }
        }

        allocateBlock();

        if (m_propertyBlocks.back()->acquireSlot(localSlotIndex))
        {
            return ((SlotsPerBlock * static_cast<uint32_t>(m_propertyBlocks.size())) + localSlotIndex);
        }
        else
        {
            // This shouldn't be possible, but log just in case ...
            logWarning("Failed to acquire new material slot.");
            return Constants::uint32_null_index;
        }
    }

    size_t MaterialProperties::totalMemoryRequirements() const noexcept
    {
        return (m_slotBytes * SlotsPerBlock * m_propertyBlocks.size());
    }
}