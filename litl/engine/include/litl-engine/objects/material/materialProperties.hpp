#ifndef LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__

#include <cstdint>
#include <memory>
#include <vector>

namespace litl
{
    struct MaterialPropertySlot
    {
        uint32_t lastActiveFrame = 0u;
        bool active = false;
    };

    struct MaterialPropertyBlock
    {
        /// <summary>
        /// The raw byte data stored within this block.
        /// This is copied directly over to the GPU.
        /// </summary>
        std::vector<std::byte> data;

        /// <summary>
        /// The frame each slot was last active during.
        /// Used to detect inactive slots that can be cleared.
        /// </summary>
        std::vector<MaterialPropertySlot> slotLastActive;

        /// <summary>
        /// The number of vacant slots in the block.
        /// </summary>
        uint32_t vacantSlotCount = 0u;

        /// <summary>
        /// Set to true if there has been a data chance to the block and its contents need to copied to the GPU.
        /// </summary>
        bool isDirty = false;

        [[nodiscard]] bool acquireSlot(uint32_t& localSlotIndex) noexcept;
    };

    /// <summary>
    /// 
    /// </summary>
    class MaterialProperties
    {
    public:

        static constexpr uint32_t SlotsPerBlock = 64u;

        /// <summary>
        /// Configures the underlying property blocks to accomodate slots of the specified byte size.
        /// </summary>
        void configure(uint32_t slotSize) noexcept;

        /// <summary>
        /// Allocates a slot in the underlying material property blocks for a new material instance to use.
        /// If slot allocation somehow fails, then the uint32 null index will be returned.
        /// </summary>
        [[nodiscard]] uint32_t allocateSlot() noexcept;

        /// <summary>
        /// Total bytes required to store all slots across all blocks. Used to determine if the GPU buffer needs to be resized.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] size_t totalMemoryRequirements() const noexcept;

    private:

        /// <summary>
        /// Allocates a new block.
        /// </summary>
        void allocateBlock() noexcept;

        std::vector<std::unique_ptr<MaterialPropertyBlock>> m_propertyBlocks;
        uint32_t m_slotBytes = 0u;
        uint32_t m_vacantSlotCount = 0u;
    };
}

#endif