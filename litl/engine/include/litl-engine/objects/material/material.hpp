#ifndef LITL_ENGINE_MATERIAL_H__
#define LITL_ENGINE_MATERIAL_H__

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "litl-core/authority.hpp"
#include "litl-core/stringId.hpp"
#include "litl-core/math/geometry/vertex.hpp"
#include "litl-engine/objects/objectDescriptor.hpp"
#include "litl-engine/objects/objectHandles.hpp"
#include "litl-engine/objects/material/materialPropertySlotId.hpp"
#include "litl-renderer/resources/graphicsPipeline.hpp"
#include "litl-renderer/resources/computePipeline.hpp"

namespace litl
{
    class MaterialManager;
    class ObjectPool;
    class Renderer;
    struct ActiveMaterialSystem;

    struct VertexInputDescriptor
    {
        static constexpr uint32_t MaxVertexAttributes = 8u;

        /// <summary>
        /// Size of the vertex structure.
        /// </summary>
        uint32_t vertexSize = sizeof(Vertex);

        /// <summary>
        /// The data format of each field of the vertex structure.
        /// </summary>
        std::array<DataFormat, MaxVertexAttributes> attributes = {
            DataFormat::RGB32_SFloat,                   // position
            DataFormat::RG32_SFloat,                    // texcoord
            DataFormat::RGB32_SFloat,                   // normal
            DataFormat::RGBA32_SFloat                   // tangent
        };
    };

    struct ShaderResourceDescriptor
    {
        std::string resource{};
        std::string entryPoint{};
        std::span<std::byte const> bytes{};
    };

    struct MaterialDescriptor
    {
        ObjectDescriptor objectInfo{};
        RasterizationState rasterizerState{};
        VertexInputDescriptor inputDescriptor{};
        ShaderResourceDescriptor vertexShader{};
        ShaderResourceDescriptor fragmentShader{};
        ShaderResourceDescriptor geometryShader{};
        ShaderResourceDescriptor tessellationControlShader{};
        ShaderResourceDescriptor tessellationEvaluationShader{};
        ShaderResourceDescriptor computeShader{};
        ShaderResourceDescriptor meshShader{};
        ShaderResourceDescriptor taskShader{};
    };

    class Material
    {
    public:

        /// <summary>
        /// The required name of the material properties variable in the PushConstant struct.
        /// </summary>
        static constexpr std::string_view MaterialPropertiesStructName = "materialProperties";
        static constexpr StringId MaterialPropertiesStructHashedName = StringId(MaterialPropertiesStructName);

        Material();
        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;
        ~Material();

        bool create(Authority<ObjectPool> auth, MaterialDescriptor const& descriptor, Renderer const& renderer, ObjectPool& objectPool) noexcept;
        void setSelfHandle(Authority<ObjectPool> author, MaterialHandle handle) noexcept;
        void destroy(Authority<ObjectPool> auth) noexcept;

        /// <summary>
        /// Retrieves the Graphics Pipeline associated with this material.
        /// </summary>
        [[nodiscard]] GraphicsPipelineHandle getGraphicsPipelineHandle() const noexcept;

        /// <summary>
        /// Retrieves the Compute Pipeline associated with this material.
        /// </summary>
        [[nodiscard]] ComputePipelineHandle getComputePipelineHandle() const noexcept;

        /// <summary>
        /// Retrieves the handle of the underlying GPU Buffer which stores graphics property data.
        /// </summary>
        [[nodiscard]] GpuBufferHandle getGraphicsGpuBufferHandle() const noexcept;

        /// <summary>
        /// Retrieves the BDA of the underlying GPU Buffer for the current frame.
        /// </summary>
        [[nodiscard]] std::optional<uint64_t> getGraphicsBufferDeviceAddress() const noexcept;

        /// <summary>
        /// Allocates a slot in the material buffer. This slot may be a reclaimed or new slot.
        /// 
        /// The slot may be marked as "frequently updated" if it is expected to be updated at a much
        /// higher frequency than what is standard for instances of the material. The use-case is for 
        /// material properties that are typically static but an event causes it to start updating on a per-frame basis.
        /// In these cases, it is then more performant to mark the slot as frequently updated. 
        /// 
        /// A real-world example would be a bomb material that is typically static but then starts flashing before exploding.
        /// 
        /// Slots marked as frequent updaters do not flag their entire block when they get modified.
        /// Instead their data is copied over to a specialized block at the end of the buffer which is copied each frame by default.
        /// This avoids a few sparsely placed, but rapidly updating, material instances from marking all of their blocks dirty causing excessive data copies.
        /// Frequently updated slots also do not have a stable (up-to-date) material slot and one must be requested each frame via getFrequentUpdateSlot.
        /// 
        /// If it is normal for instances of the material to be frequently updated, then individual slots should not
        /// be marked as such and can instead be detrimental to performance.
        /// 
        /// The caller assumes the responsibility for updating any associated MaterialRef or similar.
        /// </summary>
        /// <param name="frequentUpdates"></param>
        /// <returns></returns>
        [[nodiscard]] MaterialPropertySlotId allocateSlot(bool frequentUpdates = false) noexcept;

        /// <summary>
        /// Invoked once-per-frame to provide the material with the current frame count and index.
        /// These are used in part to help track inactive slots, dirty blocks, etc.
        /// </summary>
        void onFrameStart(Authority<MaterialManager> auth, uint32_t frame, uint32_t frameInFlightIndex) noexcept;

        /// <summary>
        /// Invoked once-per-frame to ensure the frame-current underlying GPU buffer is up-to-date.
        /// </summary>
        void onPreRender(Authority<MaterialManager> auth) noexcept;

        /// <summary>
        /// Invoked once-per-frame for all entities who have a MaterialRef component.
        /// This is used to determine which material slots are no longer in use. Any slot that is deemed
        /// inactive over a given number of frames will be automatically freed.
        /// </summary>
        void markActive(Authority<ActiveMaterialSystem> auth, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Updates the slot to be flagged as frequently updated. 
        /// The caller assumes the responsibility for updating any associated MaterialRef or similar.
        /// 
        /// Note that this action is NOT thread-safe if immediate is true.
        /// </summary>
        /// <param name="immediate">If true, the change will be immediate. Otherwise it will be added to the deffered material command queue and performed on the next onPreRender.</param>
        /// <returns>Always returns true. Used to simply pass into MaterialRef::frequentUpdates</returns>
        bool markAsFrequentUpdate(MaterialPropertySlotId slot, bool immediate = false) noexcept;

        /// <summary>
        /// Updates the slot to no longer be flagged as frequently updated. 
        /// The caller assumes the responsibility for updating any associated MaterialRef or similar.
        /// 
        /// Note that this action is NOT thread-safe if immediate is true.
        /// </summary>
        /// <param name="immediate">If true, the change will be immediate. Otherwise it will be added to the deffered material command queue and performed on the next onPreRender.</param>
        /// <returns>Always returns false. Used to simply pass into MaterialRef::frequentUpdates</returns>
        bool markAsInfrequentUpdate(MaterialPropertySlotId slot, bool immediate = false) noexcept;

        /// <summary>
        /// Given a material slot, returns the global slot index into its frequent block data.
        /// If the slot is not a resident of the frequent update block, then Constants::uint32_null_index is returned instead.
        /// </summary>
        uint32_t getFrequentUpdateSlot(MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Returns true if the material is ready to be bound for rendering.
        /// </summary>
        [[nodiscard]] bool ready() const noexcept;

        /// <summary>
        /// Sets the boolean value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setBool(StringId property, bool value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit signed integer value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setInt32(StringId property, int32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit unsigned integer value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setUint32(StringId property, uint32_t value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 32-bit float value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setFloat(StringId property, float value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 64-bit float value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setDouble(StringId property, double value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the two-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec2(StringId property, vec2 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec3(StringId property, vec3 value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the four-component 32-bit float vector value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setVec4(StringId property, vec4 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the three- or four-component 32-bit float vector value with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setColor(StringId property, color const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 3x3 32-bit float matrix value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat3(StringId property, mat3 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the 4x4 32-bit float matrix value at with the specified property name at the provided slot index.
        /// May return false if there was an error setting the value (type mismatch, invalid slot, etc.).
        /// </summary>
        bool setMat4(StringId property, mat4 const& value, MaterialPropertySlotId slot) noexcept;

        /// <summary>
        /// Sets the default bool value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultBool(StringId property, bool value) noexcept;

        /// <summary>
        /// Sets the default 32-bit signed integer value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultInt32(StringId property, int32_t value) noexcept;

        /// <summary>
        /// Sets the default 32-bit unsigned integer value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultUint32(StringId property, uint32_t value) noexcept;

        /// <summary>
        /// Sets the default 32-bit float value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultFloat(StringId property, float value) noexcept;

        /// <summary>
        /// Sets the default 64-bit float value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultDouble(StringId property, double value) noexcept;

        /// <summary>
        /// Sets the default vec2 value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultVec2(StringId property, vec2 value) noexcept;

        /// <summary>
        /// Sets the default vec3 value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultVec3(StringId property, vec3 value) noexcept;

        /// <summary>
        /// Sets the default vec4 value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultVec4(StringId property, vec4 const& value) noexcept;

        /// <summary>
        /// Sets the default color value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultColor(StringId property, color const& value) noexcept;

        /// <summary>
        /// Sets the default mat3 value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultMat3(StringId property, mat3 const& value) noexcept;

        /// <summary>
        /// Sets the default mat4 value for the specified property.
        /// This value will be applied to all slots that have do have a custom value set.
        /// </summary>
        bool setDefaultMat4(StringId property, mat4 const& value) noexcept;

    private:

        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif