#ifndef LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__
#define LITL_ENGINE_OBJECTS_MATERIAL_PROPERTIES_H__

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "litl-core/constants.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/types.hpp"
#include "litl-renderer/reflection.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"

namespace litl
{
    struct MaterialPropertySlot
    {
        /// <summary>
        /// Current version/generation of the slot.
        /// This is incremented everytime the slot is returned in an allocation and when it is freed.
        /// </summary>
        uint32_t version = 0u;

        /// <summary>
        /// The last frame that this slot was marked active.
        /// </summary>
        uint32_t lastActiveFrame = 0u;

        /// <summary>
        /// The last frame that this slot was written to.
        /// Used for demotion from the frequent block.
        /// </summary>
        uint32_t lastWriteFrame = 0u;

        /// <summary>
        /// The number of frames in a row that this slot was written to.
        /// Used for promotion to the frequent block.
        /// </summary>
        uint32_t consecutiveWriteFrames = 0u;

        /// <summary>
        /// If the instance tied to this slot is marked as a being frequently updated, this is the
        /// global slot index into the frequent update scratch buffer. So if isFrequent is true,
        /// then frequentGlobalSlot should be used instead of the MaterialPropertySlotId::index for
        /// properly indexing into the GPU buffer.
        /// 
        /// It should be noted also that this slot index is not stable and can change every frame.
        /// </summary>
        uint32_t frequentGlobalSlot = Constants::uint32_null_index;

        /// <summary>
        /// Is there an active material instance associated with this slot?
        /// </summary>
        bool occupied = false;

        /// <summary>
        /// Is this slot also represented in the frequent update block?
        /// </summary>
        bool isInFrequentUpdateBlock = false;
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

        [[nodiscard]] bool acquireSlot(uint32_t slotSize, uint32_t frame, uint32_t framesInFlight, uint32_t& localSlotIndex, uint32_t& localSlotVersion, std::span<std::byte const> defaultBlob) noexcept;
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
        std::span<std::byte const> sourcePtr{};
        uint32_t blockOffset{ 0u };
    };

    /// <summary>
    /// A block at the end of all other blocks that stores the data for those material instances which label themselves as frequent (per-frame) updaters.
    /// </summary>
    struct FrequentUpdateBlock
    {
        /// <summary>
        /// The data blob for the frequently updated instances.
        /// </summary>
        std::vector<std::byte> data;

        /// <summary>
        /// The standard block index for those instances that are also in the frequent update block.
        /// </summary>
        std::vector<uint32_t> residents;

        void removeResident(uint32_t resident) noexcept;
    };

    /// <summary>
    /// Manages the CPU-side memory for material properties.
    /// Composed of individual blocks of SlotsPerBlock slots. Each slot correlates to a material instance and contains the complete property data.
    /// All data is stored generically as byte blobs and are uploaded to the GPU buffer by the owning Material.
    /// 
    /// There are three tiers of data within MaterialProperties, with the first two tiers being stored differently than the third.
    /// 
    /// Tier 1: Property data that is static. It uses either the material defaults or values that are set only one time.
    /// Tier 2: Property data that is updated infrequently with at minimum SlotUpgradeToFrequentFrames frames between.
    /// Tier 3: Property data that is updated frequently, effectively every frame.
    /// 
    /// Data in the underlying GPU buffer, which is owned by Material and not MaterialProperties, is split into two parts.
    /// All slots have an entry into the first part of the buffer. Within MaterialProperties this is modelled as the m_propertyBlocks.
    /// This first part is written in blocks/chunks and only when one or more slots in the block is dirty.
    /// 
    /// The second part of the GPU buffer is for the frequently updated (Tier 3) properties and is rewritten each frame.
    /// This serves as a way to split frequently updated, but sparsely populated, data into its own section of the buffer
    /// and avoids over-copying of data to the GPU.
    /// 
    /// For example, if a game has 1000 entities all using the same material but only 3 of those have properties changing each
    /// frame (such as a bomb flashing before it explodes), then if the slots were in Tier 1/Tier 2 we would be uploaded up to
    /// three blocks of data each frame. But if the slots are instead marked for Tier 3, then we are only uploading those 3 slots.
    /// That is a difference of up to 189 slots of data (when SlotsPerBlock == 64).
    /// 
    /// However Tier 3 should be avoided if it is expected that a majority of instances of the material will be frequently updating.
    /// This can be controlled in the parent Material::toggleFrequentDataUpdateSeparation.
    /// </summary>
    class MaterialProperties
    {
    public:

        static constexpr uint32_t SlotsPerBlock = 64u;
        static constexpr uint32_t SlotExpirationFrames = 8u;
        static constexpr uint32_t SlotUpgradeToFrequentFrames = 8u;
        static constexpr uint32_t SlotDowngradeFromFrequntFrames = 30u;

        /// <summary>
        /// Configures the underlying property blocks to accomodate slots of the specified byte size.
        /// </summary>
        bool configure(MaterialPropertyReflection const& reflectedProperties, uint32_t framesInFlight) noexcept;

        /// <summary>
        /// Enables/disables the split of Tier 3 data into its own separate GPU buffer space.
        /// This should be enabled (and is by default) for materials that as a whole are infrequently updated
        /// but may sporadically have instances that need to update frequently on a near per-frame basis.
        /// 
        /// For example, a bomb material that is normally static but can be triggered to start flashing.
        /// </summary>
        /// <param name="enabled"></param>
        void toggleTier3DataSeparation(bool enabled) noexcept;

        /// <summary>
        /// Sets the handle of the owning material.
        /// </summary>
        void setMaterialHandle(MaterialHandle handle) noexcept;

        /// <summary>
        /// Sets the default property values assigned when a new slot is allocated.
        /// May return false if the new blob does not equal the size of the current blob.
        /// </summary>
        [[nodiscard]] bool setDefaultPropertyBlob(std::span<std::byte const> defaultBlob) noexcept;

        /// <summary>
        /// Sets the current frame that is being rendered. This is used to track which slots are actively being used.
        /// </summary>
        void setCurrentFrame(uint32_t currFrame) noexcept;

        /// <summary>
        /// Allocates a slot in the underlying material property blocks for a new material instance to use.
        /// If slot allocation somehow fails, then the uint32 null index will be returned.
        /// </summary>
        [[nodiscard]] MaterialPropertySlotId allocateSlot() noexcept;

        /// <summary>
        /// Returns the index into the GPU buffer that the provided slot resides in.
        /// This value can not be considered stable as it may reside in the frequent write block which gets recreated each frame.
        /// </summary>
        [[nodiscard]] uint32_t getSlotIndex(MaterialPropertySlotId slotId) const noexcept;

        /// <summary>
        /// Marks the slot active for the frame.
        /// 
        /// Note this does not update the block active count as this method is intended be called from 
        /// a parallel ECS system run, and so we split out the active slot count calculation to avoid thread syncing mechanisms.
        /// </summary>
        void markSlotActive(MaterialPropertySlotId slot) noexcept;
        
        /// <summary>
        /// Marks all blocks dirty and that they need to be reuploaded to the GPU.
        /// Typically used when the underlying GPU buffers were resized and need to be updated.
        /// </summary>
        void markAllBlocksDirty() noexcept;

        /// <summary>
        /// Given a material slot, returns the global slot index into its frequent block data.
        /// If the slot is not a resident of the frequent update block, then Constants::uint32_null_index is returned instead.
        /// </summary>
        [[nodiscard]] uint32_t getFrequentUpdateSlot(MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Called each frame to rebuild the frequent update block.
        /// </summary>
        void rebuildFrequentUpdateBlock() noexcept;

        /// <summary>
        /// 
        /// </summary>
        void upgradeSlotToFrequentBlock(MaterialPropertySlotId slotId) noexcept;

        /// <summary>
        /// Retrieves the pointer information for the frequent update block. Returns false if the update block has no active residents.
        /// </summary>
        [[nodiscard]] bool gatherFrequentUpdateBlockPointer(MaterialPropertyBlockPointer& blockPointer) noexcept;

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
        bool setBool(StringId property, bool value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 32-bit signed integer value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setInt32(StringId property, int32_t value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 32-bit unsigned integer value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 32-bit float value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setFloat(StringId property, float value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 64-bit float value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setDouble(StringId property, double value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the two-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec2(StringId property, vec2 value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the three-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec3(StringId property, vec3 value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the three- or four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setColor(StringId property, color const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 3x3 32-bit float matrix value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Sets the 4x4 32-bit float matrix value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot, bool defaultValue) noexcept;

    private:

        /// <summary>
        /// Allocates a new block.
        /// </summary>
        void allocateBlock() noexcept;

        /// <summary>
        /// Given a global slot index, resolves it to a block index and local slot index into the block.
        /// May return false if the global index is out-of-bounds.
        /// </summary>
        [[nodiscard]] bool getBlockLocalSlot(MaterialPropertySlotId slotId , uint32_t& blockIndex, uint32_t& localSlot, uint32_t& slotVersion, bool validateVersion) const noexcept;

        /// <summary>
        /// Retrieves the direct pointer to the specified slot data. Returns null if the provided slot id is out-of-bounds, stale, or otherwise invalid.
        /// </summary>
        [[nodiscard]] std::byte* getSlotDataPtr(MaterialPropertySlotId slotId, uint32_t& blockIndex, uint32_t& localSlot, bool validateVersion) const noexcept;

        /// <summary>
        /// Given a property string id, returns the associated ResourceProperty. If no match was found, returns null.
        /// </summary>
        ResourceProperty const* getReflectedProperty(StringId property) const noexcept;

        /// <summary>
        /// Generic data set for a single property in a block. All other set methods (setBool, setFloat, setColor, etc.) all flow into here.
        /// </summary>
        bool setData(uint32_t propertyOffset, uint32_t propertySize, void const* propertyData, MaterialPropertySlotId slot, bool defaultValue) noexcept;

        /// <summary>
        /// Size of an individual slot in a block.
        /// </summary>
        uint32_t m_slotSizeBytes = 0u;

        /// <summary>
        /// The current frame render frame. 
        /// Used to track which slots are active.
        /// </summary>
        uint32_t m_currFrame = 0u;

        /// <summary>
        /// Number of frames-in-flight managed by the renderer. 
        /// When a block is marked dirty, it is dirty for this number of frames so that all 
        /// frame-specific copies of the underlying buffer can be updated with the new data.
        /// </summary>
        uint32_t m_framesInFlight = 0u;

        /// <summary>
        /// The reflected layout of all properties in an individual slot.
        /// </summary>
        std::vector<ResourceProperty> m_properties;

        /// <summary>
        /// Maps from a property name id to the local offset within an individual slot.
        /// </summary>
        StringIdMap<uint32_t> m_propertyMap;

        /// <summary>
        /// All property blocks. Each block holds SlotsPerBlock number of slots.
        /// When a slot is updated, the entire block is marked dirty and will be uploaded to the GPU at the next update.
        /// </summary>
        std::vector<std::unique_ptr<MaterialPropertyBlock>> m_propertyBlocks;

        /// <summary>
        /// Provided by the material, the default values of a new slot.
        /// </summary>
        std::vector<std::byte> m_defaultPropertyBlob;

        /// <summary>
        /// Provided by the material, the default values of a new slot.
        /// </summary>
        FrequentUpdateBlock m_frequentUpdateBlock;

        /// <summary>
        /// Handle of the owning material.
        /// </summary>
        MaterialHandle m_materialHandle{};

        /// <summary>
        /// Separate Tier 3 data in the GPU buffer.
        /// </summary>
        bool m_enabledTier3DataSeparation{ true };
    };
}

#endif