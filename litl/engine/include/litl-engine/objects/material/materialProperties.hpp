#ifndef LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__

#include <cstdint>
#include <memory>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types.hpp"
#include "litl-renderer/reflection.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    struct MaterialPropertySlot
    {
        uint32_t version = 0u;
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
        /// The last frame the block was updated.
        /// </summary>
        uint32_t lastActiveFrame = 0u;

        /// <summary>
        /// Set to the number of frames-in-flight if there has been a data chance to the block and its contents need to copied to the GPU.
        /// </summary>
        uint32_t dirtyFrameCount = 0u;

        [[nodiscard]] bool acquireSlot(uint32_t slotSize, uint32_t frame, uint32_t framesInFlight, uint32_t& localSlotIndex, uint32_t& localSlotVersion) noexcept;
    };

    struct MaterialPropertyReflection
    {
        /// <summary>
        /// The size in bytes of an individual material property slot.
        /// </summary>
        uint32_t sizeBytes = 0u;

        /// <summary>
        /// The individual properties of the material structure.
        /// </summary>
        std::vector<ResourceProperty> properties;
    };

    /// <summary>
    /// Offset and data pointer for a dirty property block.
    /// </summary>
    struct MaterialPropertyBlockPointer
    {
        std::span<std::byte> sourcePtr{};
        uint32_t blockIndex{ 0u };
    };

    /// <summary>
    /// 
    /// </summary>
    class MaterialProperties
    {
    public:

        static constexpr uint32_t SlotsPerBlock = 64u;
        static constexpr uint32_t SlotExpirationFrames = 8u;

        /// <summary>
        /// Configures the underlying property blocks to accomodate slots of the specified byte size.
        /// </summary>
        bool configure(MaterialPropertyReflection const& reflectedProperties, uint32_t framesInFlight) noexcept;

        /// <summary>
        /// ... todo be called by something that ticks each frame ...
        /// </summary>
        void setCurrentFrame(uint32_t currFrame) noexcept;

        /// <summary>
        /// Allocates a slot in the underlying material property blocks for a new material instance to use.
        /// If slot allocation somehow fails, then the uint32 null index will be returned.
        /// </summary>
        [[nodiscard]] MaterialPropertySlotId allocateSlot() noexcept;

        /// <summary>
        /// Marks the slot active for the frame.
        /// 
        /// Note this does not update the block active count as this method is intended be called from 
        /// a parallel ECS system run, and so we split out the active slot count calculation to avoid thread syncing mechanisms.
        /// 
        /// ... todo be called by a system or something ....
        /// </summary>
        void markSlotActive(MaterialPropertySlotId slot) noexcept;
        
        /// <summary>
        /// Marks all blocks dirty and that they need to be reuploaded to the GPU.
        /// Typically used when the underlying GPU buffers were resized and need to be updated.
        /// </summary>
        void markAllBlocksDirty() noexcept;

        /// <summary>
        /// Frees all slots found to be inactive.
        /// </summary>
        void freeSlots() noexcept;

        /// <summary>
        /// Retrieves the number of bytes needed to store an individual slot.
        /// </summary>
        [[nodiscard]] uint32_t individualSlotMemoryRequirements() const noexcept;

        /// <summary>
        /// Total bytes required to store all slots across all blocks. Used to determine if the GPU buffer needs to be resized.
        /// </summary>
        [[nodiscard]] size_t totalMemoryRequirements() const noexcept;

        /// <summary>
        /// Returns the number of properties.
        /// </summary>
        [[nodiscard]] uint32_t propertyCount() const noexcept;

        /// <summary>
        /// Retrieves the indices of all blocks marked dirty.
        /// </summary>
        void gatherDirtyBlocks(std::vector<MaterialPropertyBlockPointer>& dirtyBlocks) noexcept;

        /// <summary>
        /// Marks all blocks as no longer dirty.
        /// </summary>
        void clearDirtyBlocks() noexcept;

        /// <summary>
        /// Sets the boolean value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setBool(StringId property, bool value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit signed integer value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit unsigned integer value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit float value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 64-bit float value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the two-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three- or four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setColor(StringId property, color const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 3x3 32-bit float matrix value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 4x4 32-bit float matrix value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot) noexcept;

    private:

        /// <summary>
        /// Allocates a new block.
        /// </summary>
        void allocateBlock() noexcept;

        /// <summary>
        /// Given a global slot index, resolves it to a block index and local slot index into the block.
        /// May return false if the global index is out-of-bounds.
        /// </summary>
        [[nodiscard]] bool getBlockLocalSlot(uint32_t slot, uint32_t& blockIndex, uint32_t& localSlot, uint32_t& localSlotVersion) const noexcept;

        /// <summary>
        /// Given a property string id, returns the associated ResourceProperty. If no match was found, returns null.
        /// </summary>
        ResourceProperty const* getReflectedProperty(StringId property) const noexcept;

        /// <summary>
        /// 
        /// </summary>
        bool setData(uint32_t propertyOffset, uint32_t propertySize, void const* propertyData, MaterialPropertySlotId slot) noexcept;

        uint32_t m_currFrame = 0u;
        uint32_t m_elementSizeBytes = 0u;
        uint32_t m_framesInFlight = 0u;

        std::vector<ResourceProperty> m_properties;
        StringIdMap<uint32_t> m_propertyMap;
        std::vector<std::unique_ptr<MaterialPropertyBlock>> m_propertyBlocks;
    };
}

#endif