#ifndef LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__

#include "litl-core/stringId.hpp"
#include "litl-core/math/types.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace litl
{
    struct MaterialPropertySlot
    {
        uint32_t lastActiveFrame = 0u;
        bool occupied = false;
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
        std::vector<MaterialPropertySlot> slots;

        /// <summary>
        /// The number of vacant slots in the block.
        /// </summary>
        uint32_t vacantSlotCount = 0u;

        /// <summary>
        /// The number of slots marked active this past frame.
        /// </summary>
        uint32_t activeSlotCount = 0u;

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
        /// Marks the slot active for the frame.
        /// </summary>
        void markSlotActive(uint32_t slot, uint32_t frame) noexcept;

        /// <summary>
        /// Frees all slots found to be inactive.
        /// </summary>
        void freeSlots(uint32_t frame) noexcept;

        /// <summary>
        /// Total bytes required to store all slots across all blocks. Used to determine if the GPU buffer needs to be resized.
        /// </summary>
        /// <returns></returns>
        [[nodiscard]] size_t totalMemoryRequirements() const noexcept;

        bool setBool(StringId property, bool value, uint32_t slot) noexcept;
        bool setInt32(StringId property, int32_t value, uint32_t slot) noexcept;
        bool setUint32(StringId property, uint32_t value, uint32_t slot) noexcept;
        bool setFloat(StringId property, float value, uint32_t slot) noexcept;
        bool setVec2(StringId property, vec2 value, uint32_t slot) noexcept;
        bool setVec3(StringId property, vec3 value, uint32_t slot) noexcept;
        bool setVec4(StringId property, vec4 const& value, uint32_t slot) noexcept;
        bool setMat3(StringId property, mat3 const& value, uint32_t slot) noexcept;
        bool setMat4(StringId property, mat4 const& value, uint32_t slot) noexcept;

    private:

        /// <summary>
        /// Allocates a new block.
        /// </summary>
        void allocateBlock() noexcept;

        /// <summary>
        /// Given a global slot index, resolves it to a block index and local slot index into the block.
        /// May return false if the global index is out-of-bounds.
        /// </summary>
        [[nodiscard]] bool getBlockLocalSlot(uint32_t slot, uint32_t& blockIndex, uint32_t& localSlot) const noexcept;

        bool setData(StringId property, uint32_t propertyBytes, void* propertyData, uint32_t slot) noexcept;

        StringIdMap<uint32_t> m_propertyOffsets;
        std::vector<std::unique_ptr<MaterialPropertyBlock>> m_propertyBlocks;
        uint32_t m_slotBytes = 0u;
        uint32_t m_vacantSlotCount = 0u;
    };
}

#endif